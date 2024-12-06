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

#include "bcm4912_drivers_xport_ag.h"
#include "bcm4912_xport_portreset_ag.h"
#define BLOCK_ADDR_COUNT_BITS 1
#define BLOCK_ADDR_COUNT (1<<BLOCK_ADDR_COUNT_BITS)

int ag_drv_xport_portreset_p0_ctrl_set(uint8_t xlmac_id, uint8_t port_sw_reset)
{
    uint32_t reg_p0_ctrl=0;

#ifdef VALIDATE_PARMS
    if((xlmac_id >= BLOCK_ADDR_COUNT) ||
       (port_sw_reset >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_p0_ctrl = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P0_CTRL, PORT_SW_RESET, reg_p0_ctrl, port_sw_reset);

    RU_REG_WRITE(xlmac_id, XPORT_PORTRESET, P0_CTRL, reg_p0_ctrl);

    return 0;
}

int ag_drv_xport_portreset_p0_ctrl_get(uint8_t xlmac_id, uint8_t *port_sw_reset)
{
    uint32_t reg_p0_ctrl=0;

#ifdef VALIDATE_PARMS
    if(!port_sw_reset)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(xlmac_id, XPORT_PORTRESET, P0_CTRL, reg_p0_ctrl);

    *port_sw_reset = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P0_CTRL, PORT_SW_RESET, reg_p0_ctrl);

    return 0;
}

int ag_drv_xport_portreset_p1_ctrl_set(uint8_t xlmac_id, uint8_t port_sw_reset)
{
    uint32_t reg_p1_ctrl=0;

#ifdef VALIDATE_PARMS
    if((xlmac_id >= BLOCK_ADDR_COUNT) ||
       (port_sw_reset >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_p1_ctrl = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P1_CTRL, PORT_SW_RESET, reg_p1_ctrl, port_sw_reset);

    RU_REG_WRITE(xlmac_id, XPORT_PORTRESET, P1_CTRL, reg_p1_ctrl);

    return 0;
}

int ag_drv_xport_portreset_p1_ctrl_get(uint8_t xlmac_id, uint8_t *port_sw_reset)
{
    uint32_t reg_p1_ctrl=0;

#ifdef VALIDATE_PARMS
    if(!port_sw_reset)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(xlmac_id, XPORT_PORTRESET, P1_CTRL, reg_p1_ctrl);

    *port_sw_reset = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P1_CTRL, PORT_SW_RESET, reg_p1_ctrl);

    return 0;
}

int ag_drv_xport_portreset_p2_ctrl_set(uint8_t xlmac_id, uint8_t port_sw_reset)
{
    uint32_t reg_p2_ctrl=0;

#ifdef VALIDATE_PARMS
    if((xlmac_id >= BLOCK_ADDR_COUNT) ||
       (port_sw_reset >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_p2_ctrl = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P2_CTRL, PORT_SW_RESET, reg_p2_ctrl, port_sw_reset);

    RU_REG_WRITE(xlmac_id, XPORT_PORTRESET, P2_CTRL, reg_p2_ctrl);

    return 0;
}

int ag_drv_xport_portreset_p2_ctrl_get(uint8_t xlmac_id, uint8_t *port_sw_reset)
{
    uint32_t reg_p2_ctrl=0;

#ifdef VALIDATE_PARMS
    if(!port_sw_reset)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(xlmac_id, XPORT_PORTRESET, P2_CTRL, reg_p2_ctrl);

    *port_sw_reset = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P2_CTRL, PORT_SW_RESET, reg_p2_ctrl);

    return 0;
}

int ag_drv_xport_portreset_p3_ctrl_set(uint8_t xlmac_id, uint8_t port_sw_reset)
{
    uint32_t reg_p3_ctrl=0;

#ifdef VALIDATE_PARMS
    if((xlmac_id >= BLOCK_ADDR_COUNT) ||
       (port_sw_reset >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_p3_ctrl = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P3_CTRL, PORT_SW_RESET, reg_p3_ctrl, port_sw_reset);

    RU_REG_WRITE(xlmac_id, XPORT_PORTRESET, P3_CTRL, reg_p3_ctrl);

    return 0;
}

int ag_drv_xport_portreset_p3_ctrl_get(uint8_t xlmac_id, uint8_t *port_sw_reset)
{
    uint32_t reg_p3_ctrl=0;

#ifdef VALIDATE_PARMS
    if(!port_sw_reset)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(xlmac_id, XPORT_PORTRESET, P3_CTRL, reg_p3_ctrl);

    *port_sw_reset = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P3_CTRL, PORT_SW_RESET, reg_p3_ctrl);

    return 0;
}

int ag_drv_xport_portreset_config_set(uint8_t xlmac_id, uint8_t link_down_rst_en, uint8_t enable_sm_run, uint16_t tick_timer_ndiv)
{
    uint32_t reg_config=0;

#ifdef VALIDATE_PARMS
    if((xlmac_id >= BLOCK_ADDR_COUNT) ||
       (link_down_rst_en >= _4BITS_MAX_VAL_) ||
       (enable_sm_run >= _4BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_config = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, CONFIG, LINK_DOWN_RST_EN, reg_config, link_down_rst_en);
    reg_config = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, CONFIG, ENABLE_SM_RUN, reg_config, enable_sm_run);
    reg_config = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, CONFIG, TICK_TIMER_NDIV, reg_config, tick_timer_ndiv);

    RU_REG_WRITE(xlmac_id, XPORT_PORTRESET, CONFIG, reg_config);

    return 0;
}

int ag_drv_xport_portreset_config_get(uint8_t xlmac_id, uint8_t *link_down_rst_en, uint8_t *enable_sm_run, uint16_t *tick_timer_ndiv)
{
    uint32_t reg_config=0;

#ifdef VALIDATE_PARMS
    if(!link_down_rst_en || !enable_sm_run || !tick_timer_ndiv)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(xlmac_id, XPORT_PORTRESET, CONFIG, reg_config);

    *link_down_rst_en = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, CONFIG, LINK_DOWN_RST_EN, reg_config);
    *enable_sm_run = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, CONFIG, ENABLE_SM_RUN, reg_config);
    *tick_timer_ndiv = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, CONFIG, TICK_TIMER_NDIV, reg_config);

    return 0;
}

int ag_drv_xport_portreset_p0_link_stat_debounce_cfg_set(uint8_t xlmac_id, uint8_t disable, uint16_t debounce_time)
{
    uint32_t reg_p0_link_stat_debounce_cfg=0;

#ifdef VALIDATE_PARMS
    if((xlmac_id >= BLOCK_ADDR_COUNT) ||
       (disable >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_p0_link_stat_debounce_cfg = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P0_LINK_STAT_DEBOUNCE_CFG, DISABLE, reg_p0_link_stat_debounce_cfg, disable);
    reg_p0_link_stat_debounce_cfg = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P0_LINK_STAT_DEBOUNCE_CFG, DEBOUNCE_TIME, reg_p0_link_stat_debounce_cfg, debounce_time);

    RU_REG_WRITE(xlmac_id, XPORT_PORTRESET, P0_LINK_STAT_DEBOUNCE_CFG, reg_p0_link_stat_debounce_cfg);

    return 0;
}

int ag_drv_xport_portreset_p0_link_stat_debounce_cfg_get(uint8_t xlmac_id, uint8_t *disable, uint16_t *debounce_time)
{
    uint32_t reg_p0_link_stat_debounce_cfg=0;

#ifdef VALIDATE_PARMS
    if(!disable || !debounce_time)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(xlmac_id, XPORT_PORTRESET, P0_LINK_STAT_DEBOUNCE_CFG, reg_p0_link_stat_debounce_cfg);

    *disable = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P0_LINK_STAT_DEBOUNCE_CFG, DISABLE, reg_p0_link_stat_debounce_cfg);
    *debounce_time = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P0_LINK_STAT_DEBOUNCE_CFG, DEBOUNCE_TIME, reg_p0_link_stat_debounce_cfg);

    return 0;
}

int ag_drv_xport_portreset_p1_link_stat_debounce_cfg_set(uint8_t xlmac_id, uint8_t disable, uint16_t debounce_time)
{
    uint32_t reg_p1_link_stat_debounce_cfg=0;

#ifdef VALIDATE_PARMS
    if((xlmac_id >= BLOCK_ADDR_COUNT) ||
       (disable >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_p1_link_stat_debounce_cfg = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P1_LINK_STAT_DEBOUNCE_CFG, DISABLE, reg_p1_link_stat_debounce_cfg, disable);
    reg_p1_link_stat_debounce_cfg = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P1_LINK_STAT_DEBOUNCE_CFG, DEBOUNCE_TIME, reg_p1_link_stat_debounce_cfg, debounce_time);

    RU_REG_WRITE(xlmac_id, XPORT_PORTRESET, P1_LINK_STAT_DEBOUNCE_CFG, reg_p1_link_stat_debounce_cfg);

    return 0;
}

int ag_drv_xport_portreset_p1_link_stat_debounce_cfg_get(uint8_t xlmac_id, uint8_t *disable, uint16_t *debounce_time)
{
    uint32_t reg_p1_link_stat_debounce_cfg=0;

#ifdef VALIDATE_PARMS
    if(!disable || !debounce_time)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(xlmac_id, XPORT_PORTRESET, P1_LINK_STAT_DEBOUNCE_CFG, reg_p1_link_stat_debounce_cfg);

    *disable = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P1_LINK_STAT_DEBOUNCE_CFG, DISABLE, reg_p1_link_stat_debounce_cfg);
    *debounce_time = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P1_LINK_STAT_DEBOUNCE_CFG, DEBOUNCE_TIME, reg_p1_link_stat_debounce_cfg);

    return 0;
}

int ag_drv_xport_portreset_p2_link_stat_debounce_cfg_set(uint8_t xlmac_id, uint8_t disable, uint16_t debounce_time)
{
    uint32_t reg_p2_link_stat_debounce_cfg=0;

#ifdef VALIDATE_PARMS
    if((xlmac_id >= BLOCK_ADDR_COUNT) ||
       (disable >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_p2_link_stat_debounce_cfg = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P2_LINK_STAT_DEBOUNCE_CFG, DISABLE, reg_p2_link_stat_debounce_cfg, disable);
    reg_p2_link_stat_debounce_cfg = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P2_LINK_STAT_DEBOUNCE_CFG, DEBOUNCE_TIME, reg_p2_link_stat_debounce_cfg, debounce_time);

    RU_REG_WRITE(xlmac_id, XPORT_PORTRESET, P2_LINK_STAT_DEBOUNCE_CFG, reg_p2_link_stat_debounce_cfg);

    return 0;
}

int ag_drv_xport_portreset_p2_link_stat_debounce_cfg_get(uint8_t xlmac_id, uint8_t *disable, uint16_t *debounce_time)
{
    uint32_t reg_p2_link_stat_debounce_cfg=0;

#ifdef VALIDATE_PARMS
    if(!disable || !debounce_time)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(xlmac_id, XPORT_PORTRESET, P2_LINK_STAT_DEBOUNCE_CFG, reg_p2_link_stat_debounce_cfg);

    *disable = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P2_LINK_STAT_DEBOUNCE_CFG, DISABLE, reg_p2_link_stat_debounce_cfg);
    *debounce_time = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P2_LINK_STAT_DEBOUNCE_CFG, DEBOUNCE_TIME, reg_p2_link_stat_debounce_cfg);

    return 0;
}

int ag_drv_xport_portreset_p3_link_stat_debounce_cfg_set(uint8_t xlmac_id, uint8_t disable, uint16_t debounce_time)
{
    uint32_t reg_p3_link_stat_debounce_cfg=0;

#ifdef VALIDATE_PARMS
    if((xlmac_id >= BLOCK_ADDR_COUNT) ||
       (disable >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_p3_link_stat_debounce_cfg = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P3_LINK_STAT_DEBOUNCE_CFG, DISABLE, reg_p3_link_stat_debounce_cfg, disable);
    reg_p3_link_stat_debounce_cfg = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P3_LINK_STAT_DEBOUNCE_CFG, DEBOUNCE_TIME, reg_p3_link_stat_debounce_cfg, debounce_time);

    RU_REG_WRITE(xlmac_id, XPORT_PORTRESET, P3_LINK_STAT_DEBOUNCE_CFG, reg_p3_link_stat_debounce_cfg);

    return 0;
}

int ag_drv_xport_portreset_p3_link_stat_debounce_cfg_get(uint8_t xlmac_id, uint8_t *disable, uint16_t *debounce_time)
{
    uint32_t reg_p3_link_stat_debounce_cfg=0;

#ifdef VALIDATE_PARMS
    if(!disable || !debounce_time)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(xlmac_id, XPORT_PORTRESET, P3_LINK_STAT_DEBOUNCE_CFG, reg_p3_link_stat_debounce_cfg);

    *disable = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P3_LINK_STAT_DEBOUNCE_CFG, DISABLE, reg_p3_link_stat_debounce_cfg);
    *debounce_time = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P3_LINK_STAT_DEBOUNCE_CFG, DEBOUNCE_TIME, reg_p3_link_stat_debounce_cfg);

    return 0;
}

int ag_drv_xport_portreset_p0_sig_en_set(uint8_t xlmac_id, const xport_portreset_sig_en *p0_sig_en)
{
    uint32_t reg_p0_sig_en=0;

#ifdef VALIDATE_PARMS
    if(!p0_sig_en)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT) ||
       (p0_sig_en->enable_xlmac_rx_disab >= _1BITS_MAX_VAL_) ||
       (p0_sig_en->enable_xlmac_tx_disab >= _1BITS_MAX_VAL_) ||
       (p0_sig_en->enable_xlmac_tx_discard >= _1BITS_MAX_VAL_) ||
       (p0_sig_en->enable_xlmac_soft_reset >= _1BITS_MAX_VAL_) ||
       (p0_sig_en->enable_mab_rx_port_init >= _1BITS_MAX_VAL_) ||
       (p0_sig_en->enable_mab_tx_port_init >= _1BITS_MAX_VAL_) ||
       (p0_sig_en->enable_mab_tx_credit_disab >= _1BITS_MAX_VAL_) ||
       (p0_sig_en->enable_mab_tx_fifo_init >= _1BITS_MAX_VAL_) ||
       (p0_sig_en->enable_port_is_under_reset >= _1BITS_MAX_VAL_) ||
       (p0_sig_en->enable_xlmac_ep_discard >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_p0_sig_en = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P0_SIG_EN, ENABLE_XLMAC_RX_DISAB, reg_p0_sig_en, p0_sig_en->enable_xlmac_rx_disab);
    reg_p0_sig_en = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P0_SIG_EN, ENABLE_XLMAC_TX_DISAB, reg_p0_sig_en, p0_sig_en->enable_xlmac_tx_disab);
    reg_p0_sig_en = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P0_SIG_EN, ENABLE_XLMAC_TX_DISCARD, reg_p0_sig_en, p0_sig_en->enable_xlmac_tx_discard);
    reg_p0_sig_en = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P0_SIG_EN, ENABLE_XLMAC_SOFT_RESET, reg_p0_sig_en, p0_sig_en->enable_xlmac_soft_reset);
    reg_p0_sig_en = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P0_SIG_EN, ENABLE_MAB_RX_PORT_INIT, reg_p0_sig_en, p0_sig_en->enable_mab_rx_port_init);
    reg_p0_sig_en = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P0_SIG_EN, ENABLE_MAB_TX_PORT_INIT, reg_p0_sig_en, p0_sig_en->enable_mab_tx_port_init);
    reg_p0_sig_en = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P0_SIG_EN, ENABLE_MAB_TX_CREDIT_DISAB, reg_p0_sig_en, p0_sig_en->enable_mab_tx_credit_disab);
    reg_p0_sig_en = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P0_SIG_EN, ENABLE_MAB_TX_FIFO_INIT, reg_p0_sig_en, p0_sig_en->enable_mab_tx_fifo_init);
    reg_p0_sig_en = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P0_SIG_EN, ENABLE_PORT_IS_UNDER_RESET, reg_p0_sig_en, p0_sig_en->enable_port_is_under_reset);
    reg_p0_sig_en = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P0_SIG_EN, ENABLE_XLMAC_EP_DISCARD, reg_p0_sig_en, p0_sig_en->enable_xlmac_ep_discard);

    RU_REG_WRITE(xlmac_id, XPORT_PORTRESET, P0_SIG_EN, reg_p0_sig_en);

    return 0;
}

int ag_drv_xport_portreset_p0_sig_en_get(uint8_t xlmac_id, xport_portreset_sig_en *p0_sig_en)
{
    uint32_t reg_p0_sig_en=0;

#ifdef VALIDATE_PARMS
    if(!p0_sig_en)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(xlmac_id, XPORT_PORTRESET, P0_SIG_EN, reg_p0_sig_en);

    p0_sig_en->enable_xlmac_rx_disab = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P0_SIG_EN, ENABLE_XLMAC_RX_DISAB, reg_p0_sig_en);
    p0_sig_en->enable_xlmac_tx_disab = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P0_SIG_EN, ENABLE_XLMAC_TX_DISAB, reg_p0_sig_en);
    p0_sig_en->enable_xlmac_tx_discard = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P0_SIG_EN, ENABLE_XLMAC_TX_DISCARD, reg_p0_sig_en);
    p0_sig_en->enable_xlmac_soft_reset = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P0_SIG_EN, ENABLE_XLMAC_SOFT_RESET, reg_p0_sig_en);
    p0_sig_en->enable_mab_rx_port_init = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P0_SIG_EN, ENABLE_MAB_RX_PORT_INIT, reg_p0_sig_en);
    p0_sig_en->enable_mab_tx_port_init = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P0_SIG_EN, ENABLE_MAB_TX_PORT_INIT, reg_p0_sig_en);
    p0_sig_en->enable_mab_tx_credit_disab = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P0_SIG_EN, ENABLE_MAB_TX_CREDIT_DISAB, reg_p0_sig_en);
    p0_sig_en->enable_mab_tx_fifo_init = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P0_SIG_EN, ENABLE_MAB_TX_FIFO_INIT, reg_p0_sig_en);
    p0_sig_en->enable_port_is_under_reset = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P0_SIG_EN, ENABLE_PORT_IS_UNDER_RESET, reg_p0_sig_en);
    p0_sig_en->enable_xlmac_ep_discard = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P0_SIG_EN, ENABLE_XLMAC_EP_DISCARD, reg_p0_sig_en);

    return 0;
}

int ag_drv_xport_portreset_p0_sig_assert_times_0_set(uint8_t xlmac_id, uint16_t xlmac_rx_disab_assert_time, uint16_t xlmac_tx_disab_assert_time)
{
    uint32_t reg_p0_sig_assert_times_0=0;

#ifdef VALIDATE_PARMS
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_p0_sig_assert_times_0 = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P0_SIG_ASSERT_TIMES_0, XLMAC_RX_DISAB_ASSERT_TIME, reg_p0_sig_assert_times_0, xlmac_rx_disab_assert_time);
    reg_p0_sig_assert_times_0 = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P0_SIG_ASSERT_TIMES_0, XLMAC_TX_DISAB_ASSERT_TIME, reg_p0_sig_assert_times_0, xlmac_tx_disab_assert_time);

    RU_REG_WRITE(xlmac_id, XPORT_PORTRESET, P0_SIG_ASSERT_TIMES_0, reg_p0_sig_assert_times_0);

    return 0;
}

int ag_drv_xport_portreset_p0_sig_assert_times_0_get(uint8_t xlmac_id, uint16_t *xlmac_rx_disab_assert_time, uint16_t *xlmac_tx_disab_assert_time)
{
    uint32_t reg_p0_sig_assert_times_0=0;

#ifdef VALIDATE_PARMS
    if(!xlmac_rx_disab_assert_time || !xlmac_tx_disab_assert_time)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(xlmac_id, XPORT_PORTRESET, P0_SIG_ASSERT_TIMES_0, reg_p0_sig_assert_times_0);

    *xlmac_rx_disab_assert_time = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P0_SIG_ASSERT_TIMES_0, XLMAC_RX_DISAB_ASSERT_TIME, reg_p0_sig_assert_times_0);
    *xlmac_tx_disab_assert_time = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P0_SIG_ASSERT_TIMES_0, XLMAC_TX_DISAB_ASSERT_TIME, reg_p0_sig_assert_times_0);

    return 0;
}

int ag_drv_xport_portreset_p0_sig_assert_times_1_set(uint8_t xlmac_id, uint16_t xlmac_txdiscard_assert_time, uint16_t xlmac_soft_reset_assert_time)
{
    uint32_t reg_p0_sig_assert_times_1=0;

#ifdef VALIDATE_PARMS
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_p0_sig_assert_times_1 = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P0_SIG_ASSERT_TIMES_1, XLMAC_TXDISCARD_ASSERT_TIME, reg_p0_sig_assert_times_1, xlmac_txdiscard_assert_time);
    reg_p0_sig_assert_times_1 = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P0_SIG_ASSERT_TIMES_1, XLMAC_SOFT_RESET_ASSERT_TIME, reg_p0_sig_assert_times_1, xlmac_soft_reset_assert_time);

    RU_REG_WRITE(xlmac_id, XPORT_PORTRESET, P0_SIG_ASSERT_TIMES_1, reg_p0_sig_assert_times_1);

    return 0;
}

int ag_drv_xport_portreset_p0_sig_assert_times_1_get(uint8_t xlmac_id, uint16_t *xlmac_txdiscard_assert_time, uint16_t *xlmac_soft_reset_assert_time)
{
    uint32_t reg_p0_sig_assert_times_1=0;

#ifdef VALIDATE_PARMS
    if(!xlmac_txdiscard_assert_time || !xlmac_soft_reset_assert_time)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(xlmac_id, XPORT_PORTRESET, P0_SIG_ASSERT_TIMES_1, reg_p0_sig_assert_times_1);

    *xlmac_txdiscard_assert_time = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P0_SIG_ASSERT_TIMES_1, XLMAC_TXDISCARD_ASSERT_TIME, reg_p0_sig_assert_times_1);
    *xlmac_soft_reset_assert_time = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P0_SIG_ASSERT_TIMES_1, XLMAC_SOFT_RESET_ASSERT_TIME, reg_p0_sig_assert_times_1);

    return 0;
}

int ag_drv_xport_portreset_p0_sig_assert_times_2_set(uint8_t xlmac_id, uint16_t mab_rx_port_init_assert_time, uint16_t mab_tx_port_init_assert_time)
{
    uint32_t reg_p0_sig_assert_times_2=0;

#ifdef VALIDATE_PARMS
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_p0_sig_assert_times_2 = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P0_SIG_ASSERT_TIMES_2, MAB_RX_PORT_INIT_ASSERT_TIME, reg_p0_sig_assert_times_2, mab_rx_port_init_assert_time);
    reg_p0_sig_assert_times_2 = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P0_SIG_ASSERT_TIMES_2, MAB_TX_PORT_INIT_ASSERT_TIME, reg_p0_sig_assert_times_2, mab_tx_port_init_assert_time);

    RU_REG_WRITE(xlmac_id, XPORT_PORTRESET, P0_SIG_ASSERT_TIMES_2, reg_p0_sig_assert_times_2);

    return 0;
}

int ag_drv_xport_portreset_p0_sig_assert_times_2_get(uint8_t xlmac_id, uint16_t *mab_rx_port_init_assert_time, uint16_t *mab_tx_port_init_assert_time)
{
    uint32_t reg_p0_sig_assert_times_2=0;

#ifdef VALIDATE_PARMS
    if(!mab_rx_port_init_assert_time || !mab_tx_port_init_assert_time)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(xlmac_id, XPORT_PORTRESET, P0_SIG_ASSERT_TIMES_2, reg_p0_sig_assert_times_2);

    *mab_rx_port_init_assert_time = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P0_SIG_ASSERT_TIMES_2, MAB_RX_PORT_INIT_ASSERT_TIME, reg_p0_sig_assert_times_2);
    *mab_tx_port_init_assert_time = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P0_SIG_ASSERT_TIMES_2, MAB_TX_PORT_INIT_ASSERT_TIME, reg_p0_sig_assert_times_2);

    return 0;
}

int ag_drv_xport_portreset_p0_sig_assert_times_3_set(uint8_t xlmac_id, uint16_t mab_tx_credit_disab_assert_time, uint16_t mab_tx_fifo_init_assert_time)
{
    uint32_t reg_p0_sig_assert_times_3=0;

#ifdef VALIDATE_PARMS
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_p0_sig_assert_times_3 = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P0_SIG_ASSERT_TIMES_3, MAB_TX_CREDIT_DISAB_ASSERT_TIME, reg_p0_sig_assert_times_3, mab_tx_credit_disab_assert_time);
    reg_p0_sig_assert_times_3 = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P0_SIG_ASSERT_TIMES_3, MAB_TX_FIFO_INIT_ASSERT_TIME, reg_p0_sig_assert_times_3, mab_tx_fifo_init_assert_time);

    RU_REG_WRITE(xlmac_id, XPORT_PORTRESET, P0_SIG_ASSERT_TIMES_3, reg_p0_sig_assert_times_3);

    return 0;
}

int ag_drv_xport_portreset_p0_sig_assert_times_3_get(uint8_t xlmac_id, uint16_t *mab_tx_credit_disab_assert_time, uint16_t *mab_tx_fifo_init_assert_time)
{
    uint32_t reg_p0_sig_assert_times_3=0;

#ifdef VALIDATE_PARMS
    if(!mab_tx_credit_disab_assert_time || !mab_tx_fifo_init_assert_time)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(xlmac_id, XPORT_PORTRESET, P0_SIG_ASSERT_TIMES_3, reg_p0_sig_assert_times_3);

    *mab_tx_credit_disab_assert_time = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P0_SIG_ASSERT_TIMES_3, MAB_TX_CREDIT_DISAB_ASSERT_TIME, reg_p0_sig_assert_times_3);
    *mab_tx_fifo_init_assert_time = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P0_SIG_ASSERT_TIMES_3, MAB_TX_FIFO_INIT_ASSERT_TIME, reg_p0_sig_assert_times_3);

    return 0;
}

int ag_drv_xport_portreset_p0_sig_assert_times_4_set(uint8_t xlmac_id, uint16_t port_is_under_reset_assert_time)
{
    uint32_t reg_p0_sig_assert_times_4=0;

#ifdef VALIDATE_PARMS
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_p0_sig_assert_times_4 = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P0_SIG_ASSERT_TIMES_4, PORT_IS_UNDER_RESET_ASSERT_TIME, reg_p0_sig_assert_times_4, port_is_under_reset_assert_time);

    RU_REG_WRITE(xlmac_id, XPORT_PORTRESET, P0_SIG_ASSERT_TIMES_4, reg_p0_sig_assert_times_4);

    return 0;
}

int ag_drv_xport_portreset_p0_sig_assert_times_4_get(uint8_t xlmac_id, uint16_t *port_is_under_reset_assert_time)
{
    uint32_t reg_p0_sig_assert_times_4=0;

#ifdef VALIDATE_PARMS
    if(!port_is_under_reset_assert_time)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(xlmac_id, XPORT_PORTRESET, P0_SIG_ASSERT_TIMES_4, reg_p0_sig_assert_times_4);

    *port_is_under_reset_assert_time = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P0_SIG_ASSERT_TIMES_4, PORT_IS_UNDER_RESET_ASSERT_TIME, reg_p0_sig_assert_times_4);

    return 0;
}

int ag_drv_xport_portreset_p0_sig_deassert_times_0_set(uint8_t xlmac_id, uint16_t xlmac_rx_disab_deassert_time, uint16_t xlmac_tx_disab_deassert_time)
{
    uint32_t reg_p0_sig_deassert_times_0=0;

#ifdef VALIDATE_PARMS
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_p0_sig_deassert_times_0 = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P0_SIG_DEASSERT_TIMES_0, XLMAC_RX_DISAB_DEASSERT_TIME, reg_p0_sig_deassert_times_0, xlmac_rx_disab_deassert_time);
    reg_p0_sig_deassert_times_0 = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P0_SIG_DEASSERT_TIMES_0, XLMAC_TX_DISAB_DEASSERT_TIME, reg_p0_sig_deassert_times_0, xlmac_tx_disab_deassert_time);

    RU_REG_WRITE(xlmac_id, XPORT_PORTRESET, P0_SIG_DEASSERT_TIMES_0, reg_p0_sig_deassert_times_0);

    return 0;
}

int ag_drv_xport_portreset_p0_sig_deassert_times_0_get(uint8_t xlmac_id, uint16_t *xlmac_rx_disab_deassert_time, uint16_t *xlmac_tx_disab_deassert_time)
{
    uint32_t reg_p0_sig_deassert_times_0=0;

#ifdef VALIDATE_PARMS
    if(!xlmac_rx_disab_deassert_time || !xlmac_tx_disab_deassert_time)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(xlmac_id, XPORT_PORTRESET, P0_SIG_DEASSERT_TIMES_0, reg_p0_sig_deassert_times_0);

    *xlmac_rx_disab_deassert_time = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P0_SIG_DEASSERT_TIMES_0, XLMAC_RX_DISAB_DEASSERT_TIME, reg_p0_sig_deassert_times_0);
    *xlmac_tx_disab_deassert_time = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P0_SIG_DEASSERT_TIMES_0, XLMAC_TX_DISAB_DEASSERT_TIME, reg_p0_sig_deassert_times_0);

    return 0;
}

int ag_drv_xport_portreset_p0_sig_deassert_times_1_set(uint8_t xlmac_id, uint16_t xlmac_txdiscard_deassert_time, uint16_t xlmac_soft_reset_deassert_time)
{
    uint32_t reg_p0_sig_deassert_times_1=0;

#ifdef VALIDATE_PARMS
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_p0_sig_deassert_times_1 = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P0_SIG_DEASSERT_TIMES_1, XLMAC_TXDISCARD_DEASSERT_TIME, reg_p0_sig_deassert_times_1, xlmac_txdiscard_deassert_time);
    reg_p0_sig_deassert_times_1 = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P0_SIG_DEASSERT_TIMES_1, XLMAC_SOFT_RESET_DEASSERT_TIME, reg_p0_sig_deassert_times_1, xlmac_soft_reset_deassert_time);

    RU_REG_WRITE(xlmac_id, XPORT_PORTRESET, P0_SIG_DEASSERT_TIMES_1, reg_p0_sig_deassert_times_1);

    return 0;
}

int ag_drv_xport_portreset_p0_sig_deassert_times_1_get(uint8_t xlmac_id, uint16_t *xlmac_txdiscard_deassert_time, uint16_t *xlmac_soft_reset_deassert_time)
{
    uint32_t reg_p0_sig_deassert_times_1=0;

#ifdef VALIDATE_PARMS
    if(!xlmac_txdiscard_deassert_time || !xlmac_soft_reset_deassert_time)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(xlmac_id, XPORT_PORTRESET, P0_SIG_DEASSERT_TIMES_1, reg_p0_sig_deassert_times_1);

    *xlmac_txdiscard_deassert_time = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P0_SIG_DEASSERT_TIMES_1, XLMAC_TXDISCARD_DEASSERT_TIME, reg_p0_sig_deassert_times_1);
    *xlmac_soft_reset_deassert_time = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P0_SIG_DEASSERT_TIMES_1, XLMAC_SOFT_RESET_DEASSERT_TIME, reg_p0_sig_deassert_times_1);

    return 0;
}

int ag_drv_xport_portreset_p0_sig_deassert_times_2_set(uint8_t xlmac_id, uint16_t mab_rx_port_init_deassert_time, uint16_t mab_tx_port_init_deassert_time)
{
    uint32_t reg_p0_sig_deassert_times_2=0;

#ifdef VALIDATE_PARMS
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_p0_sig_deassert_times_2 = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P0_SIG_DEASSERT_TIMES_2, MAB_RX_PORT_INIT_DEASSERT_TIME, reg_p0_sig_deassert_times_2, mab_rx_port_init_deassert_time);
    reg_p0_sig_deassert_times_2 = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P0_SIG_DEASSERT_TIMES_2, MAB_TX_PORT_INIT_DEASSERT_TIME, reg_p0_sig_deassert_times_2, mab_tx_port_init_deassert_time);

    RU_REG_WRITE(xlmac_id, XPORT_PORTRESET, P0_SIG_DEASSERT_TIMES_2, reg_p0_sig_deassert_times_2);

    return 0;
}

int ag_drv_xport_portreset_p0_sig_deassert_times_2_get(uint8_t xlmac_id, uint16_t *mab_rx_port_init_deassert_time, uint16_t *mab_tx_port_init_deassert_time)
{
    uint32_t reg_p0_sig_deassert_times_2=0;

#ifdef VALIDATE_PARMS
    if(!mab_rx_port_init_deassert_time || !mab_tx_port_init_deassert_time)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(xlmac_id, XPORT_PORTRESET, P0_SIG_DEASSERT_TIMES_2, reg_p0_sig_deassert_times_2);

    *mab_rx_port_init_deassert_time = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P0_SIG_DEASSERT_TIMES_2, MAB_RX_PORT_INIT_DEASSERT_TIME, reg_p0_sig_deassert_times_2);
    *mab_tx_port_init_deassert_time = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P0_SIG_DEASSERT_TIMES_2, MAB_TX_PORT_INIT_DEASSERT_TIME, reg_p0_sig_deassert_times_2);

    return 0;
}

int ag_drv_xport_portreset_p0_sig_deassert_times_3_set(uint8_t xlmac_id, uint16_t mab_tx_credit_disab_deassert_time, uint16_t mab_tx_fifo_init_deassert_time)
{
    uint32_t reg_p0_sig_deassert_times_3=0;

#ifdef VALIDATE_PARMS
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_p0_sig_deassert_times_3 = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P0_SIG_DEASSERT_TIMES_3, MAB_TX_CREDIT_DISAB_DEASSERT_TIME, reg_p0_sig_deassert_times_3, mab_tx_credit_disab_deassert_time);
    reg_p0_sig_deassert_times_3 = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P0_SIG_DEASSERT_TIMES_3, MAB_TX_FIFO_INIT_DEASSERT_TIME, reg_p0_sig_deassert_times_3, mab_tx_fifo_init_deassert_time);

    RU_REG_WRITE(xlmac_id, XPORT_PORTRESET, P0_SIG_DEASSERT_TIMES_3, reg_p0_sig_deassert_times_3);

    return 0;
}

int ag_drv_xport_portreset_p0_sig_deassert_times_3_get(uint8_t xlmac_id, uint16_t *mab_tx_credit_disab_deassert_time, uint16_t *mab_tx_fifo_init_deassert_time)
{
    uint32_t reg_p0_sig_deassert_times_3=0;

#ifdef VALIDATE_PARMS
    if(!mab_tx_credit_disab_deassert_time || !mab_tx_fifo_init_deassert_time)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(xlmac_id, XPORT_PORTRESET, P0_SIG_DEASSERT_TIMES_3, reg_p0_sig_deassert_times_3);

    *mab_tx_credit_disab_deassert_time = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P0_SIG_DEASSERT_TIMES_3, MAB_TX_CREDIT_DISAB_DEASSERT_TIME, reg_p0_sig_deassert_times_3);
    *mab_tx_fifo_init_deassert_time = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P0_SIG_DEASSERT_TIMES_3, MAB_TX_FIFO_INIT_DEASSERT_TIME, reg_p0_sig_deassert_times_3);

    return 0;
}

int ag_drv_xport_portreset_p0_sig_deassert_times_4_set(uint8_t xlmac_id, uint16_t port_is_under_reset_deassert_time)
{
    uint32_t reg_p0_sig_deassert_times_4=0;

#ifdef VALIDATE_PARMS
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_p0_sig_deassert_times_4 = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P0_SIG_DEASSERT_TIMES_4, PORT_IS_UNDER_RESET_DEASSERT_TIME, reg_p0_sig_deassert_times_4, port_is_under_reset_deassert_time);

    RU_REG_WRITE(xlmac_id, XPORT_PORTRESET, P0_SIG_DEASSERT_TIMES_4, reg_p0_sig_deassert_times_4);

    return 0;
}

int ag_drv_xport_portreset_p0_sig_deassert_times_4_get(uint8_t xlmac_id, uint16_t *port_is_under_reset_deassert_time)
{
    uint32_t reg_p0_sig_deassert_times_4=0;

#ifdef VALIDATE_PARMS
    if(!port_is_under_reset_deassert_time)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(xlmac_id, XPORT_PORTRESET, P0_SIG_DEASSERT_TIMES_4, reg_p0_sig_deassert_times_4);

    *port_is_under_reset_deassert_time = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P0_SIG_DEASSERT_TIMES_4, PORT_IS_UNDER_RESET_DEASSERT_TIME, reg_p0_sig_deassert_times_4);

    return 0;
}

int ag_drv_xport_portreset_p1_sig_en_set(uint8_t xlmac_id, const xport_portreset_sig_en *p1_sig_en)
{
    uint32_t reg_p1_sig_en=0;

#ifdef VALIDATE_PARMS
    if(!p1_sig_en)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT) ||
       (p1_sig_en->enable_xlmac_rx_disab >= _1BITS_MAX_VAL_) ||
       (p1_sig_en->enable_xlmac_tx_disab >= _1BITS_MAX_VAL_) ||
       (p1_sig_en->enable_xlmac_tx_discard >= _1BITS_MAX_VAL_) ||
       (p1_sig_en->enable_xlmac_soft_reset >= _1BITS_MAX_VAL_) ||
       (p1_sig_en->enable_mab_rx_port_init >= _1BITS_MAX_VAL_) ||
       (p1_sig_en->enable_mab_tx_port_init >= _1BITS_MAX_VAL_) ||
       (p1_sig_en->enable_mab_tx_credit_disab >= _1BITS_MAX_VAL_) ||
       (p1_sig_en->enable_mab_tx_fifo_init >= _1BITS_MAX_VAL_) ||
       (p1_sig_en->enable_port_is_under_reset >= _1BITS_MAX_VAL_) ||
       (p1_sig_en->enable_xlmac_ep_discard >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_p1_sig_en = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P1_SIG_EN, ENABLE_XLMAC_RX_DISAB, reg_p1_sig_en, p1_sig_en->enable_xlmac_rx_disab);
    reg_p1_sig_en = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P1_SIG_EN, ENABLE_XLMAC_TX_DISAB, reg_p1_sig_en, p1_sig_en->enable_xlmac_tx_disab);
    reg_p1_sig_en = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P1_SIG_EN, ENABLE_XLMAC_TX_DISCARD, reg_p1_sig_en, p1_sig_en->enable_xlmac_tx_discard);
    reg_p1_sig_en = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P1_SIG_EN, ENABLE_XLMAC_SOFT_RESET, reg_p1_sig_en, p1_sig_en->enable_xlmac_soft_reset);
    reg_p1_sig_en = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P1_SIG_EN, ENABLE_MAB_RX_PORT_INIT, reg_p1_sig_en, p1_sig_en->enable_mab_rx_port_init);
    reg_p1_sig_en = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P1_SIG_EN, ENABLE_MAB_TX_PORT_INIT, reg_p1_sig_en, p1_sig_en->enable_mab_tx_port_init);
    reg_p1_sig_en = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P1_SIG_EN, ENABLE_MAB_TX_CREDIT_DISAB, reg_p1_sig_en, p1_sig_en->enable_mab_tx_credit_disab);
    reg_p1_sig_en = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P1_SIG_EN, ENABLE_MAB_TX_FIFO_INIT, reg_p1_sig_en, p1_sig_en->enable_mab_tx_fifo_init);
    reg_p1_sig_en = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P1_SIG_EN, ENABLE_PORT_IS_UNDER_RESET, reg_p1_sig_en, p1_sig_en->enable_port_is_under_reset);
    reg_p1_sig_en = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P1_SIG_EN, ENABLE_XLMAC_EP_DISCARD, reg_p1_sig_en, p1_sig_en->enable_xlmac_ep_discard);

    RU_REG_WRITE(xlmac_id, XPORT_PORTRESET, P1_SIG_EN, reg_p1_sig_en);

    return 0;
}

int ag_drv_xport_portreset_p1_sig_en_get(uint8_t xlmac_id, xport_portreset_sig_en *p1_sig_en)
{
    uint32_t reg_p1_sig_en=0;

#ifdef VALIDATE_PARMS
    if(!p1_sig_en)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(xlmac_id, XPORT_PORTRESET, P1_SIG_EN, reg_p1_sig_en);

    p1_sig_en->enable_xlmac_rx_disab = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P1_SIG_EN, ENABLE_XLMAC_RX_DISAB, reg_p1_sig_en);
    p1_sig_en->enable_xlmac_tx_disab = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P1_SIG_EN, ENABLE_XLMAC_TX_DISAB, reg_p1_sig_en);
    p1_sig_en->enable_xlmac_tx_discard = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P1_SIG_EN, ENABLE_XLMAC_TX_DISCARD, reg_p1_sig_en);
    p1_sig_en->enable_xlmac_soft_reset = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P1_SIG_EN, ENABLE_XLMAC_SOFT_RESET, reg_p1_sig_en);
    p1_sig_en->enable_mab_rx_port_init = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P1_SIG_EN, ENABLE_MAB_RX_PORT_INIT, reg_p1_sig_en);
    p1_sig_en->enable_mab_tx_port_init = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P1_SIG_EN, ENABLE_MAB_TX_PORT_INIT, reg_p1_sig_en);
    p1_sig_en->enable_mab_tx_credit_disab = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P1_SIG_EN, ENABLE_MAB_TX_CREDIT_DISAB, reg_p1_sig_en);
    p1_sig_en->enable_mab_tx_fifo_init = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P1_SIG_EN, ENABLE_MAB_TX_FIFO_INIT, reg_p1_sig_en);
    p1_sig_en->enable_port_is_under_reset = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P1_SIG_EN, ENABLE_PORT_IS_UNDER_RESET, reg_p1_sig_en);
    p1_sig_en->enable_xlmac_ep_discard = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P1_SIG_EN, ENABLE_XLMAC_EP_DISCARD, reg_p1_sig_en);

    return 0;
}

int ag_drv_xport_portreset_p1_sig_assert_times_0_set(uint8_t xlmac_id, uint16_t xlmac_rx_disab_assert_time, uint16_t xlmac_tx_disab_assert_time)
{
    uint32_t reg_p1_sig_assert_times_0=0;

#ifdef VALIDATE_PARMS
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_p1_sig_assert_times_0 = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P1_SIG_ASSERT_TIMES_0, XLMAC_RX_DISAB_ASSERT_TIME, reg_p1_sig_assert_times_0, xlmac_rx_disab_assert_time);
    reg_p1_sig_assert_times_0 = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P1_SIG_ASSERT_TIMES_0, XLMAC_TX_DISAB_ASSERT_TIME, reg_p1_sig_assert_times_0, xlmac_tx_disab_assert_time);

    RU_REG_WRITE(xlmac_id, XPORT_PORTRESET, P1_SIG_ASSERT_TIMES_0, reg_p1_sig_assert_times_0);

    return 0;
}

int ag_drv_xport_portreset_p1_sig_assert_times_0_get(uint8_t xlmac_id, uint16_t *xlmac_rx_disab_assert_time, uint16_t *xlmac_tx_disab_assert_time)
{
    uint32_t reg_p1_sig_assert_times_0=0;

#ifdef VALIDATE_PARMS
    if(!xlmac_rx_disab_assert_time || !xlmac_tx_disab_assert_time)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(xlmac_id, XPORT_PORTRESET, P1_SIG_ASSERT_TIMES_0, reg_p1_sig_assert_times_0);

    *xlmac_rx_disab_assert_time = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P1_SIG_ASSERT_TIMES_0, XLMAC_RX_DISAB_ASSERT_TIME, reg_p1_sig_assert_times_0);
    *xlmac_tx_disab_assert_time = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P1_SIG_ASSERT_TIMES_0, XLMAC_TX_DISAB_ASSERT_TIME, reg_p1_sig_assert_times_0);

    return 0;
}

int ag_drv_xport_portreset_p1_sig_assert_times_1_set(uint8_t xlmac_id, uint16_t xlmac_txdiscard_assert_time, uint16_t xlmac_soft_reset_assert_time)
{
    uint32_t reg_p1_sig_assert_times_1=0;

#ifdef VALIDATE_PARMS
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_p1_sig_assert_times_1 = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P1_SIG_ASSERT_TIMES_1, XLMAC_TXDISCARD_ASSERT_TIME, reg_p1_sig_assert_times_1, xlmac_txdiscard_assert_time);
    reg_p1_sig_assert_times_1 = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P1_SIG_ASSERT_TIMES_1, XLMAC_SOFT_RESET_ASSERT_TIME, reg_p1_sig_assert_times_1, xlmac_soft_reset_assert_time);

    RU_REG_WRITE(xlmac_id, XPORT_PORTRESET, P1_SIG_ASSERT_TIMES_1, reg_p1_sig_assert_times_1);

    return 0;
}

int ag_drv_xport_portreset_p1_sig_assert_times_1_get(uint8_t xlmac_id, uint16_t *xlmac_txdiscard_assert_time, uint16_t *xlmac_soft_reset_assert_time)
{
    uint32_t reg_p1_sig_assert_times_1=0;

#ifdef VALIDATE_PARMS
    if(!xlmac_txdiscard_assert_time || !xlmac_soft_reset_assert_time)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(xlmac_id, XPORT_PORTRESET, P1_SIG_ASSERT_TIMES_1, reg_p1_sig_assert_times_1);

    *xlmac_txdiscard_assert_time = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P1_SIG_ASSERT_TIMES_1, XLMAC_TXDISCARD_ASSERT_TIME, reg_p1_sig_assert_times_1);
    *xlmac_soft_reset_assert_time = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P1_SIG_ASSERT_TIMES_1, XLMAC_SOFT_RESET_ASSERT_TIME, reg_p1_sig_assert_times_1);

    return 0;
}

int ag_drv_xport_portreset_p1_sig_assert_times_2_set(uint8_t xlmac_id, uint16_t mab_rx_port_init_assert_time, uint16_t mab_tx_port_init_assert_time)
{
    uint32_t reg_p1_sig_assert_times_2=0;

#ifdef VALIDATE_PARMS
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_p1_sig_assert_times_2 = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P1_SIG_ASSERT_TIMES_2, MAB_RX_PORT_INIT_ASSERT_TIME, reg_p1_sig_assert_times_2, mab_rx_port_init_assert_time);
    reg_p1_sig_assert_times_2 = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P1_SIG_ASSERT_TIMES_2, MAB_TX_PORT_INIT_ASSERT_TIME, reg_p1_sig_assert_times_2, mab_tx_port_init_assert_time);

    RU_REG_WRITE(xlmac_id, XPORT_PORTRESET, P1_SIG_ASSERT_TIMES_2, reg_p1_sig_assert_times_2);

    return 0;
}

int ag_drv_xport_portreset_p1_sig_assert_times_2_get(uint8_t xlmac_id, uint16_t *mab_rx_port_init_assert_time, uint16_t *mab_tx_port_init_assert_time)
{
    uint32_t reg_p1_sig_assert_times_2=0;

#ifdef VALIDATE_PARMS
    if(!mab_rx_port_init_assert_time || !mab_tx_port_init_assert_time)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(xlmac_id, XPORT_PORTRESET, P1_SIG_ASSERT_TIMES_2, reg_p1_sig_assert_times_2);

    *mab_rx_port_init_assert_time = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P1_SIG_ASSERT_TIMES_2, MAB_RX_PORT_INIT_ASSERT_TIME, reg_p1_sig_assert_times_2);
    *mab_tx_port_init_assert_time = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P1_SIG_ASSERT_TIMES_2, MAB_TX_PORT_INIT_ASSERT_TIME, reg_p1_sig_assert_times_2);

    return 0;
}

int ag_drv_xport_portreset_p1_sig_assert_times_3_set(uint8_t xlmac_id, uint16_t mab_tx_credit_disab_assert_time, uint16_t mab_tx_fifo_init_assert_time)
{
    uint32_t reg_p1_sig_assert_times_3=0;

#ifdef VALIDATE_PARMS
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_p1_sig_assert_times_3 = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P1_SIG_ASSERT_TIMES_3, MAB_TX_CREDIT_DISAB_ASSERT_TIME, reg_p1_sig_assert_times_3, mab_tx_credit_disab_assert_time);
    reg_p1_sig_assert_times_3 = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P1_SIG_ASSERT_TIMES_3, MAB_TX_FIFO_INIT_ASSERT_TIME, reg_p1_sig_assert_times_3, mab_tx_fifo_init_assert_time);

    RU_REG_WRITE(xlmac_id, XPORT_PORTRESET, P1_SIG_ASSERT_TIMES_3, reg_p1_sig_assert_times_3);

    return 0;
}

int ag_drv_xport_portreset_p1_sig_assert_times_3_get(uint8_t xlmac_id, uint16_t *mab_tx_credit_disab_assert_time, uint16_t *mab_tx_fifo_init_assert_time)
{
    uint32_t reg_p1_sig_assert_times_3=0;

#ifdef VALIDATE_PARMS
    if(!mab_tx_credit_disab_assert_time || !mab_tx_fifo_init_assert_time)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(xlmac_id, XPORT_PORTRESET, P1_SIG_ASSERT_TIMES_3, reg_p1_sig_assert_times_3);

    *mab_tx_credit_disab_assert_time = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P1_SIG_ASSERT_TIMES_3, MAB_TX_CREDIT_DISAB_ASSERT_TIME, reg_p1_sig_assert_times_3);
    *mab_tx_fifo_init_assert_time = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P1_SIG_ASSERT_TIMES_3, MAB_TX_FIFO_INIT_ASSERT_TIME, reg_p1_sig_assert_times_3);

    return 0;
}

int ag_drv_xport_portreset_p1_sig_assert_times_4_set(uint8_t xlmac_id, uint16_t port_is_under_reset_assert_time)
{
    uint32_t reg_p1_sig_assert_times_4=0;

#ifdef VALIDATE_PARMS
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_p1_sig_assert_times_4 = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P1_SIG_ASSERT_TIMES_4, PORT_IS_UNDER_RESET_ASSERT_TIME, reg_p1_sig_assert_times_4, port_is_under_reset_assert_time);

    RU_REG_WRITE(xlmac_id, XPORT_PORTRESET, P1_SIG_ASSERT_TIMES_4, reg_p1_sig_assert_times_4);

    return 0;
}

int ag_drv_xport_portreset_p1_sig_assert_times_4_get(uint8_t xlmac_id, uint16_t *port_is_under_reset_assert_time)
{
    uint32_t reg_p1_sig_assert_times_4=0;

#ifdef VALIDATE_PARMS
    if(!port_is_under_reset_assert_time)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(xlmac_id, XPORT_PORTRESET, P1_SIG_ASSERT_TIMES_4, reg_p1_sig_assert_times_4);

    *port_is_under_reset_assert_time = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P1_SIG_ASSERT_TIMES_4, PORT_IS_UNDER_RESET_ASSERT_TIME, reg_p1_sig_assert_times_4);

    return 0;
}

int ag_drv_xport_portreset_p1_sig_deassert_times_0_set(uint8_t xlmac_id, uint16_t xlmac_rx_disab_deassert_time, uint16_t xlmac_tx_disab_deassert_time)
{
    uint32_t reg_p1_sig_deassert_times_0=0;

#ifdef VALIDATE_PARMS
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_p1_sig_deassert_times_0 = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P1_SIG_DEASSERT_TIMES_0, XLMAC_RX_DISAB_DEASSERT_TIME, reg_p1_sig_deassert_times_0, xlmac_rx_disab_deassert_time);
    reg_p1_sig_deassert_times_0 = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P1_SIG_DEASSERT_TIMES_0, XLMAC_TX_DISAB_DEASSERT_TIME, reg_p1_sig_deassert_times_0, xlmac_tx_disab_deassert_time);

    RU_REG_WRITE(xlmac_id, XPORT_PORTRESET, P1_SIG_DEASSERT_TIMES_0, reg_p1_sig_deassert_times_0);

    return 0;
}

int ag_drv_xport_portreset_p1_sig_deassert_times_0_get(uint8_t xlmac_id, uint16_t *xlmac_rx_disab_deassert_time, uint16_t *xlmac_tx_disab_deassert_time)
{
    uint32_t reg_p1_sig_deassert_times_0=0;

#ifdef VALIDATE_PARMS
    if(!xlmac_rx_disab_deassert_time || !xlmac_tx_disab_deassert_time)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(xlmac_id, XPORT_PORTRESET, P1_SIG_DEASSERT_TIMES_0, reg_p1_sig_deassert_times_0);

    *xlmac_rx_disab_deassert_time = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P1_SIG_DEASSERT_TIMES_0, XLMAC_RX_DISAB_DEASSERT_TIME, reg_p1_sig_deassert_times_0);
    *xlmac_tx_disab_deassert_time = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P1_SIG_DEASSERT_TIMES_0, XLMAC_TX_DISAB_DEASSERT_TIME, reg_p1_sig_deassert_times_0);

    return 0;
}

int ag_drv_xport_portreset_p1_sig_deassert_times_1_set(uint8_t xlmac_id, uint16_t xlmac_txdiscard_deassert_time, uint16_t xlmac_soft_reset_deassert_time)
{
    uint32_t reg_p1_sig_deassert_times_1=0;

#ifdef VALIDATE_PARMS
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_p1_sig_deassert_times_1 = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P1_SIG_DEASSERT_TIMES_1, XLMAC_TXDISCARD_DEASSERT_TIME, reg_p1_sig_deassert_times_1, xlmac_txdiscard_deassert_time);
    reg_p1_sig_deassert_times_1 = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P1_SIG_DEASSERT_TIMES_1, XLMAC_SOFT_RESET_DEASSERT_TIME, reg_p1_sig_deassert_times_1, xlmac_soft_reset_deassert_time);

    RU_REG_WRITE(xlmac_id, XPORT_PORTRESET, P1_SIG_DEASSERT_TIMES_1, reg_p1_sig_deassert_times_1);

    return 0;
}

int ag_drv_xport_portreset_p1_sig_deassert_times_1_get(uint8_t xlmac_id, uint16_t *xlmac_txdiscard_deassert_time, uint16_t *xlmac_soft_reset_deassert_time)
{
    uint32_t reg_p1_sig_deassert_times_1=0;

#ifdef VALIDATE_PARMS
    if(!xlmac_txdiscard_deassert_time || !xlmac_soft_reset_deassert_time)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(xlmac_id, XPORT_PORTRESET, P1_SIG_DEASSERT_TIMES_1, reg_p1_sig_deassert_times_1);

    *xlmac_txdiscard_deassert_time = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P1_SIG_DEASSERT_TIMES_1, XLMAC_TXDISCARD_DEASSERT_TIME, reg_p1_sig_deassert_times_1);
    *xlmac_soft_reset_deassert_time = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P1_SIG_DEASSERT_TIMES_1, XLMAC_SOFT_RESET_DEASSERT_TIME, reg_p1_sig_deassert_times_1);

    return 0;
}

int ag_drv_xport_portreset_p1_sig_deassert_times_2_set(uint8_t xlmac_id, uint16_t mab_rx_port_init_deassert_time, uint16_t mab_tx_port_init_deassert_time)
{
    uint32_t reg_p1_sig_deassert_times_2=0;

#ifdef VALIDATE_PARMS
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_p1_sig_deassert_times_2 = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P1_SIG_DEASSERT_TIMES_2, MAB_RX_PORT_INIT_DEASSERT_TIME, reg_p1_sig_deassert_times_2, mab_rx_port_init_deassert_time);
    reg_p1_sig_deassert_times_2 = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P1_SIG_DEASSERT_TIMES_2, MAB_TX_PORT_INIT_DEASSERT_TIME, reg_p1_sig_deassert_times_2, mab_tx_port_init_deassert_time);

    RU_REG_WRITE(xlmac_id, XPORT_PORTRESET, P1_SIG_DEASSERT_TIMES_2, reg_p1_sig_deassert_times_2);

    return 0;
}

int ag_drv_xport_portreset_p1_sig_deassert_times_2_get(uint8_t xlmac_id, uint16_t *mab_rx_port_init_deassert_time, uint16_t *mab_tx_port_init_deassert_time)
{
    uint32_t reg_p1_sig_deassert_times_2=0;

#ifdef VALIDATE_PARMS
    if(!mab_rx_port_init_deassert_time || !mab_tx_port_init_deassert_time)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(xlmac_id, XPORT_PORTRESET, P1_SIG_DEASSERT_TIMES_2, reg_p1_sig_deassert_times_2);

    *mab_rx_port_init_deassert_time = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P1_SIG_DEASSERT_TIMES_2, MAB_RX_PORT_INIT_DEASSERT_TIME, reg_p1_sig_deassert_times_2);
    *mab_tx_port_init_deassert_time = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P1_SIG_DEASSERT_TIMES_2, MAB_TX_PORT_INIT_DEASSERT_TIME, reg_p1_sig_deassert_times_2);

    return 0;
}

int ag_drv_xport_portreset_p1_sig_deassert_times_3_set(uint8_t xlmac_id, uint16_t mab_tx_credit_disab_deassert_time, uint16_t mab_tx_fifo_init_deassert_time)
{
    uint32_t reg_p1_sig_deassert_times_3=0;

#ifdef VALIDATE_PARMS
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_p1_sig_deassert_times_3 = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P1_SIG_DEASSERT_TIMES_3, MAB_TX_CREDIT_DISAB_DEASSERT_TIME, reg_p1_sig_deassert_times_3, mab_tx_credit_disab_deassert_time);
    reg_p1_sig_deassert_times_3 = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P1_SIG_DEASSERT_TIMES_3, MAB_TX_FIFO_INIT_DEASSERT_TIME, reg_p1_sig_deassert_times_3, mab_tx_fifo_init_deassert_time);

    RU_REG_WRITE(xlmac_id, XPORT_PORTRESET, P1_SIG_DEASSERT_TIMES_3, reg_p1_sig_deassert_times_3);

    return 0;
}

int ag_drv_xport_portreset_p1_sig_deassert_times_3_get(uint8_t xlmac_id, uint16_t *mab_tx_credit_disab_deassert_time, uint16_t *mab_tx_fifo_init_deassert_time)
{
    uint32_t reg_p1_sig_deassert_times_3=0;

#ifdef VALIDATE_PARMS
    if(!mab_tx_credit_disab_deassert_time || !mab_tx_fifo_init_deassert_time)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(xlmac_id, XPORT_PORTRESET, P1_SIG_DEASSERT_TIMES_3, reg_p1_sig_deassert_times_3);

    *mab_tx_credit_disab_deassert_time = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P1_SIG_DEASSERT_TIMES_3, MAB_TX_CREDIT_DISAB_DEASSERT_TIME, reg_p1_sig_deassert_times_3);
    *mab_tx_fifo_init_deassert_time = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P1_SIG_DEASSERT_TIMES_3, MAB_TX_FIFO_INIT_DEASSERT_TIME, reg_p1_sig_deassert_times_3);

    return 0;
}

int ag_drv_xport_portreset_p1_sig_deassert_times_4_set(uint8_t xlmac_id, uint16_t port_is_under_reset_deassert_time)
{
    uint32_t reg_p1_sig_deassert_times_4=0;

#ifdef VALIDATE_PARMS
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_p1_sig_deassert_times_4 = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P1_SIG_DEASSERT_TIMES_4, PORT_IS_UNDER_RESET_DEASSERT_TIME, reg_p1_sig_deassert_times_4, port_is_under_reset_deassert_time);

    RU_REG_WRITE(xlmac_id, XPORT_PORTRESET, P1_SIG_DEASSERT_TIMES_4, reg_p1_sig_deassert_times_4);

    return 0;
}

int ag_drv_xport_portreset_p1_sig_deassert_times_4_get(uint8_t xlmac_id, uint16_t *port_is_under_reset_deassert_time)
{
    uint32_t reg_p1_sig_deassert_times_4=0;

#ifdef VALIDATE_PARMS
    if(!port_is_under_reset_deassert_time)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(xlmac_id, XPORT_PORTRESET, P1_SIG_DEASSERT_TIMES_4, reg_p1_sig_deassert_times_4);

    *port_is_under_reset_deassert_time = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P1_SIG_DEASSERT_TIMES_4, PORT_IS_UNDER_RESET_DEASSERT_TIME, reg_p1_sig_deassert_times_4);

    return 0;
}

int ag_drv_xport_portreset_p2_sig_en_set(uint8_t xlmac_id, const xport_portreset_sig_en *p2_sig_en)
{
    uint32_t reg_p2_sig_en=0;

#ifdef VALIDATE_PARMS
    if(!p2_sig_en)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT) ||
       (p2_sig_en->enable_xlmac_rx_disab >= _1BITS_MAX_VAL_) ||
       (p2_sig_en->enable_xlmac_tx_disab >= _1BITS_MAX_VAL_) ||
       (p2_sig_en->enable_xlmac_tx_discard >= _1BITS_MAX_VAL_) ||
       (p2_sig_en->enable_xlmac_soft_reset >= _1BITS_MAX_VAL_) ||
       (p2_sig_en->enable_mab_rx_port_init >= _1BITS_MAX_VAL_) ||
       (p2_sig_en->enable_mab_tx_port_init >= _1BITS_MAX_VAL_) ||
       (p2_sig_en->enable_mab_tx_credit_disab >= _1BITS_MAX_VAL_) ||
       (p2_sig_en->enable_mab_tx_fifo_init >= _1BITS_MAX_VAL_) ||
       (p2_sig_en->enable_port_is_under_reset >= _1BITS_MAX_VAL_) ||
       (p2_sig_en->enable_xlmac_ep_discard >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_p2_sig_en = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P2_SIG_EN, ENABLE_XLMAC_RX_DISAB, reg_p2_sig_en, p2_sig_en->enable_xlmac_rx_disab);
    reg_p2_sig_en = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P2_SIG_EN, ENABLE_XLMAC_TX_DISAB, reg_p2_sig_en, p2_sig_en->enable_xlmac_tx_disab);
    reg_p2_sig_en = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P2_SIG_EN, ENABLE_XLMAC_TX_DISCARD, reg_p2_sig_en, p2_sig_en->enable_xlmac_tx_discard);
    reg_p2_sig_en = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P2_SIG_EN, ENABLE_XLMAC_SOFT_RESET, reg_p2_sig_en, p2_sig_en->enable_xlmac_soft_reset);
    reg_p2_sig_en = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P2_SIG_EN, ENABLE_MAB_RX_PORT_INIT, reg_p2_sig_en, p2_sig_en->enable_mab_rx_port_init);
    reg_p2_sig_en = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P2_SIG_EN, ENABLE_MAB_TX_PORT_INIT, reg_p2_sig_en, p2_sig_en->enable_mab_tx_port_init);
    reg_p2_sig_en = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P2_SIG_EN, ENABLE_MAB_TX_CREDIT_DISAB, reg_p2_sig_en, p2_sig_en->enable_mab_tx_credit_disab);
    reg_p2_sig_en = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P2_SIG_EN, ENABLE_MAB_TX_FIFO_INIT, reg_p2_sig_en, p2_sig_en->enable_mab_tx_fifo_init);
    reg_p2_sig_en = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P2_SIG_EN, ENABLE_PORT_IS_UNDER_RESET, reg_p2_sig_en, p2_sig_en->enable_port_is_under_reset);
    reg_p2_sig_en = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P2_SIG_EN, ENABLE_XLMAC_EP_DISCARD, reg_p2_sig_en, p2_sig_en->enable_xlmac_ep_discard);

    RU_REG_WRITE(xlmac_id, XPORT_PORTRESET, P2_SIG_EN, reg_p2_sig_en);

    return 0;
}

int ag_drv_xport_portreset_p2_sig_en_get(uint8_t xlmac_id, xport_portreset_sig_en *p2_sig_en)
{
    uint32_t reg_p2_sig_en=0;

#ifdef VALIDATE_PARMS
    if(!p2_sig_en)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(xlmac_id, XPORT_PORTRESET, P2_SIG_EN, reg_p2_sig_en);

    p2_sig_en->enable_xlmac_rx_disab = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P2_SIG_EN, ENABLE_XLMAC_RX_DISAB, reg_p2_sig_en);
    p2_sig_en->enable_xlmac_tx_disab = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P2_SIG_EN, ENABLE_XLMAC_TX_DISAB, reg_p2_sig_en);
    p2_sig_en->enable_xlmac_tx_discard = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P2_SIG_EN, ENABLE_XLMAC_TX_DISCARD, reg_p2_sig_en);
    p2_sig_en->enable_xlmac_soft_reset = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P2_SIG_EN, ENABLE_XLMAC_SOFT_RESET, reg_p2_sig_en);
    p2_sig_en->enable_mab_rx_port_init = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P2_SIG_EN, ENABLE_MAB_RX_PORT_INIT, reg_p2_sig_en);
    p2_sig_en->enable_mab_tx_port_init = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P2_SIG_EN, ENABLE_MAB_TX_PORT_INIT, reg_p2_sig_en);
    p2_sig_en->enable_mab_tx_credit_disab = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P2_SIG_EN, ENABLE_MAB_TX_CREDIT_DISAB, reg_p2_sig_en);
    p2_sig_en->enable_mab_tx_fifo_init = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P2_SIG_EN, ENABLE_MAB_TX_FIFO_INIT, reg_p2_sig_en);
    p2_sig_en->enable_port_is_under_reset = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P2_SIG_EN, ENABLE_PORT_IS_UNDER_RESET, reg_p2_sig_en);
    p2_sig_en->enable_xlmac_ep_discard = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P2_SIG_EN, ENABLE_XLMAC_EP_DISCARD, reg_p2_sig_en);

    return 0;
}

int ag_drv_xport_portreset_p2_sig_assert_times_0_set(uint8_t xlmac_id, uint16_t xlmac_rx_disab_assert_time, uint16_t xlmac_tx_disab_assert_time)
{
    uint32_t reg_p2_sig_assert_times_0=0;

#ifdef VALIDATE_PARMS
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_p2_sig_assert_times_0 = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P2_SIG_ASSERT_TIMES_0, XLMAC_RX_DISAB_ASSERT_TIME, reg_p2_sig_assert_times_0, xlmac_rx_disab_assert_time);
    reg_p2_sig_assert_times_0 = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P2_SIG_ASSERT_TIMES_0, XLMAC_TX_DISAB_ASSERT_TIME, reg_p2_sig_assert_times_0, xlmac_tx_disab_assert_time);

    RU_REG_WRITE(xlmac_id, XPORT_PORTRESET, P2_SIG_ASSERT_TIMES_0, reg_p2_sig_assert_times_0);

    return 0;
}

int ag_drv_xport_portreset_p2_sig_assert_times_0_get(uint8_t xlmac_id, uint16_t *xlmac_rx_disab_assert_time, uint16_t *xlmac_tx_disab_assert_time)
{
    uint32_t reg_p2_sig_assert_times_0=0;

#ifdef VALIDATE_PARMS
    if(!xlmac_rx_disab_assert_time || !xlmac_tx_disab_assert_time)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(xlmac_id, XPORT_PORTRESET, P2_SIG_ASSERT_TIMES_0, reg_p2_sig_assert_times_0);

    *xlmac_rx_disab_assert_time = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P2_SIG_ASSERT_TIMES_0, XLMAC_RX_DISAB_ASSERT_TIME, reg_p2_sig_assert_times_0);
    *xlmac_tx_disab_assert_time = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P2_SIG_ASSERT_TIMES_0, XLMAC_TX_DISAB_ASSERT_TIME, reg_p2_sig_assert_times_0);

    return 0;
}

int ag_drv_xport_portreset_p2_sig_assert_times_1_set(uint8_t xlmac_id, uint16_t xlmac_txdiscard_assert_time, uint16_t xlmac_soft_reset_assert_time)
{
    uint32_t reg_p2_sig_assert_times_1=0;

#ifdef VALIDATE_PARMS
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_p2_sig_assert_times_1 = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P2_SIG_ASSERT_TIMES_1, XLMAC_TXDISCARD_ASSERT_TIME, reg_p2_sig_assert_times_1, xlmac_txdiscard_assert_time);
    reg_p2_sig_assert_times_1 = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P2_SIG_ASSERT_TIMES_1, XLMAC_SOFT_RESET_ASSERT_TIME, reg_p2_sig_assert_times_1, xlmac_soft_reset_assert_time);

    RU_REG_WRITE(xlmac_id, XPORT_PORTRESET, P2_SIG_ASSERT_TIMES_1, reg_p2_sig_assert_times_1);

    return 0;
}

int ag_drv_xport_portreset_p2_sig_assert_times_1_get(uint8_t xlmac_id, uint16_t *xlmac_txdiscard_assert_time, uint16_t *xlmac_soft_reset_assert_time)
{
    uint32_t reg_p2_sig_assert_times_1=0;

#ifdef VALIDATE_PARMS
    if(!xlmac_txdiscard_assert_time || !xlmac_soft_reset_assert_time)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(xlmac_id, XPORT_PORTRESET, P2_SIG_ASSERT_TIMES_1, reg_p2_sig_assert_times_1);

    *xlmac_txdiscard_assert_time = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P2_SIG_ASSERT_TIMES_1, XLMAC_TXDISCARD_ASSERT_TIME, reg_p2_sig_assert_times_1);
    *xlmac_soft_reset_assert_time = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P2_SIG_ASSERT_TIMES_1, XLMAC_SOFT_RESET_ASSERT_TIME, reg_p2_sig_assert_times_1);

    return 0;
}

int ag_drv_xport_portreset_p2_sig_assert_times_2_set(uint8_t xlmac_id, uint16_t mab_rx_port_init_assert_time, uint16_t mab_tx_port_init_assert_time)
{
    uint32_t reg_p2_sig_assert_times_2=0;

#ifdef VALIDATE_PARMS
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_p2_sig_assert_times_2 = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P2_SIG_ASSERT_TIMES_2, MAB_RX_PORT_INIT_ASSERT_TIME, reg_p2_sig_assert_times_2, mab_rx_port_init_assert_time);
    reg_p2_sig_assert_times_2 = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P2_SIG_ASSERT_TIMES_2, MAB_TX_PORT_INIT_ASSERT_TIME, reg_p2_sig_assert_times_2, mab_tx_port_init_assert_time);

    RU_REG_WRITE(xlmac_id, XPORT_PORTRESET, P2_SIG_ASSERT_TIMES_2, reg_p2_sig_assert_times_2);

    return 0;
}

int ag_drv_xport_portreset_p2_sig_assert_times_2_get(uint8_t xlmac_id, uint16_t *mab_rx_port_init_assert_time, uint16_t *mab_tx_port_init_assert_time)
{
    uint32_t reg_p2_sig_assert_times_2=0;

#ifdef VALIDATE_PARMS
    if(!mab_rx_port_init_assert_time || !mab_tx_port_init_assert_time)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(xlmac_id, XPORT_PORTRESET, P2_SIG_ASSERT_TIMES_2, reg_p2_sig_assert_times_2);

    *mab_rx_port_init_assert_time = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P2_SIG_ASSERT_TIMES_2, MAB_RX_PORT_INIT_ASSERT_TIME, reg_p2_sig_assert_times_2);
    *mab_tx_port_init_assert_time = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P2_SIG_ASSERT_TIMES_2, MAB_TX_PORT_INIT_ASSERT_TIME, reg_p2_sig_assert_times_2);

    return 0;
}

int ag_drv_xport_portreset_p2_sig_assert_times_3_set(uint8_t xlmac_id, uint16_t mab_tx_credit_disab_assert_time, uint16_t mab_tx_fifo_init_assert_time)
{
    uint32_t reg_p2_sig_assert_times_3=0;

#ifdef VALIDATE_PARMS
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_p2_sig_assert_times_3 = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P2_SIG_ASSERT_TIMES_3, MAB_TX_CREDIT_DISAB_ASSERT_TIME, reg_p2_sig_assert_times_3, mab_tx_credit_disab_assert_time);
    reg_p2_sig_assert_times_3 = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P2_SIG_ASSERT_TIMES_3, MAB_TX_FIFO_INIT_ASSERT_TIME, reg_p2_sig_assert_times_3, mab_tx_fifo_init_assert_time);

    RU_REG_WRITE(xlmac_id, XPORT_PORTRESET, P2_SIG_ASSERT_TIMES_3, reg_p2_sig_assert_times_3);

    return 0;
}

int ag_drv_xport_portreset_p2_sig_assert_times_3_get(uint8_t xlmac_id, uint16_t *mab_tx_credit_disab_assert_time, uint16_t *mab_tx_fifo_init_assert_time)
{
    uint32_t reg_p2_sig_assert_times_3=0;

#ifdef VALIDATE_PARMS
    if(!mab_tx_credit_disab_assert_time || !mab_tx_fifo_init_assert_time)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(xlmac_id, XPORT_PORTRESET, P2_SIG_ASSERT_TIMES_3, reg_p2_sig_assert_times_3);

    *mab_tx_credit_disab_assert_time = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P2_SIG_ASSERT_TIMES_3, MAB_TX_CREDIT_DISAB_ASSERT_TIME, reg_p2_sig_assert_times_3);
    *mab_tx_fifo_init_assert_time = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P2_SIG_ASSERT_TIMES_3, MAB_TX_FIFO_INIT_ASSERT_TIME, reg_p2_sig_assert_times_3);

    return 0;
}

int ag_drv_xport_portreset_p2_sig_assert_times_4_set(uint8_t xlmac_id, uint16_t port_is_under_reset_assert_time)
{
    uint32_t reg_p2_sig_assert_times_4=0;

#ifdef VALIDATE_PARMS
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_p2_sig_assert_times_4 = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P2_SIG_ASSERT_TIMES_4, PORT_IS_UNDER_RESET_ASSERT_TIME, reg_p2_sig_assert_times_4, port_is_under_reset_assert_time);

    RU_REG_WRITE(xlmac_id, XPORT_PORTRESET, P2_SIG_ASSERT_TIMES_4, reg_p2_sig_assert_times_4);

    return 0;
}

int ag_drv_xport_portreset_p2_sig_assert_times_4_get(uint8_t xlmac_id, uint16_t *port_is_under_reset_assert_time)
{
    uint32_t reg_p2_sig_assert_times_4=0;

#ifdef VALIDATE_PARMS
    if(!port_is_under_reset_assert_time)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(xlmac_id, XPORT_PORTRESET, P2_SIG_ASSERT_TIMES_4, reg_p2_sig_assert_times_4);

    *port_is_under_reset_assert_time = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P2_SIG_ASSERT_TIMES_4, PORT_IS_UNDER_RESET_ASSERT_TIME, reg_p2_sig_assert_times_4);

    return 0;
}

int ag_drv_xport_portreset_p2_sig_deassert_times_0_set(uint8_t xlmac_id, uint16_t xlmac_rx_disab_deassert_time, uint16_t xlmac_tx_disab_deassert_time)
{
    uint32_t reg_p2_sig_deassert_times_0=0;

#ifdef VALIDATE_PARMS
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_p2_sig_deassert_times_0 = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P2_SIG_DEASSERT_TIMES_0, XLMAC_RX_DISAB_DEASSERT_TIME, reg_p2_sig_deassert_times_0, xlmac_rx_disab_deassert_time);
    reg_p2_sig_deassert_times_0 = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P2_SIG_DEASSERT_TIMES_0, XLMAC_TX_DISAB_DEASSERT_TIME, reg_p2_sig_deassert_times_0, xlmac_tx_disab_deassert_time);

    RU_REG_WRITE(xlmac_id, XPORT_PORTRESET, P2_SIG_DEASSERT_TIMES_0, reg_p2_sig_deassert_times_0);

    return 0;
}

int ag_drv_xport_portreset_p2_sig_deassert_times_0_get(uint8_t xlmac_id, uint16_t *xlmac_rx_disab_deassert_time, uint16_t *xlmac_tx_disab_deassert_time)
{
    uint32_t reg_p2_sig_deassert_times_0=0;

#ifdef VALIDATE_PARMS
    if(!xlmac_rx_disab_deassert_time || !xlmac_tx_disab_deassert_time)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(xlmac_id, XPORT_PORTRESET, P2_SIG_DEASSERT_TIMES_0, reg_p2_sig_deassert_times_0);

    *xlmac_rx_disab_deassert_time = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P2_SIG_DEASSERT_TIMES_0, XLMAC_RX_DISAB_DEASSERT_TIME, reg_p2_sig_deassert_times_0);
    *xlmac_tx_disab_deassert_time = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P2_SIG_DEASSERT_TIMES_0, XLMAC_TX_DISAB_DEASSERT_TIME, reg_p2_sig_deassert_times_0);

    return 0;
}

int ag_drv_xport_portreset_p2_sig_deassert_times_1_set(uint8_t xlmac_id, uint16_t xlmac_txdiscard_deassert_time, uint16_t xlmac_soft_reset_deassert_time)
{
    uint32_t reg_p2_sig_deassert_times_1=0;

#ifdef VALIDATE_PARMS
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_p2_sig_deassert_times_1 = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P2_SIG_DEASSERT_TIMES_1, XLMAC_TXDISCARD_DEASSERT_TIME, reg_p2_sig_deassert_times_1, xlmac_txdiscard_deassert_time);
    reg_p2_sig_deassert_times_1 = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P2_SIG_DEASSERT_TIMES_1, XLMAC_SOFT_RESET_DEASSERT_TIME, reg_p2_sig_deassert_times_1, xlmac_soft_reset_deassert_time);

    RU_REG_WRITE(xlmac_id, XPORT_PORTRESET, P2_SIG_DEASSERT_TIMES_1, reg_p2_sig_deassert_times_1);

    return 0;
}

int ag_drv_xport_portreset_p2_sig_deassert_times_1_get(uint8_t xlmac_id, uint16_t *xlmac_txdiscard_deassert_time, uint16_t *xlmac_soft_reset_deassert_time)
{
    uint32_t reg_p2_sig_deassert_times_1=0;

#ifdef VALIDATE_PARMS
    if(!xlmac_txdiscard_deassert_time || !xlmac_soft_reset_deassert_time)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(xlmac_id, XPORT_PORTRESET, P2_SIG_DEASSERT_TIMES_1, reg_p2_sig_deassert_times_1);

    *xlmac_txdiscard_deassert_time = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P2_SIG_DEASSERT_TIMES_1, XLMAC_TXDISCARD_DEASSERT_TIME, reg_p2_sig_deassert_times_1);
    *xlmac_soft_reset_deassert_time = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P2_SIG_DEASSERT_TIMES_1, XLMAC_SOFT_RESET_DEASSERT_TIME, reg_p2_sig_deassert_times_1);

    return 0;
}

int ag_drv_xport_portreset_p2_sig_deassert_times_2_set(uint8_t xlmac_id, uint16_t mab_rx_port_init_deassert_time, uint16_t mab_tx_port_init_deassert_time)
{
    uint32_t reg_p2_sig_deassert_times_2=0;

#ifdef VALIDATE_PARMS
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_p2_sig_deassert_times_2 = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P2_SIG_DEASSERT_TIMES_2, MAB_RX_PORT_INIT_DEASSERT_TIME, reg_p2_sig_deassert_times_2, mab_rx_port_init_deassert_time);
    reg_p2_sig_deassert_times_2 = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P2_SIG_DEASSERT_TIMES_2, MAB_TX_PORT_INIT_DEASSERT_TIME, reg_p2_sig_deassert_times_2, mab_tx_port_init_deassert_time);

    RU_REG_WRITE(xlmac_id, XPORT_PORTRESET, P2_SIG_DEASSERT_TIMES_2, reg_p2_sig_deassert_times_2);

    return 0;
}

int ag_drv_xport_portreset_p2_sig_deassert_times_2_get(uint8_t xlmac_id, uint16_t *mab_rx_port_init_deassert_time, uint16_t *mab_tx_port_init_deassert_time)
{
    uint32_t reg_p2_sig_deassert_times_2=0;

#ifdef VALIDATE_PARMS
    if(!mab_rx_port_init_deassert_time || !mab_tx_port_init_deassert_time)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(xlmac_id, XPORT_PORTRESET, P2_SIG_DEASSERT_TIMES_2, reg_p2_sig_deassert_times_2);

    *mab_rx_port_init_deassert_time = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P2_SIG_DEASSERT_TIMES_2, MAB_RX_PORT_INIT_DEASSERT_TIME, reg_p2_sig_deassert_times_2);
    *mab_tx_port_init_deassert_time = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P2_SIG_DEASSERT_TIMES_2, MAB_TX_PORT_INIT_DEASSERT_TIME, reg_p2_sig_deassert_times_2);

    return 0;
}

int ag_drv_xport_portreset_p2_sig_deassert_times_3_set(uint8_t xlmac_id, uint16_t mab_tx_credit_disab_deassert_time, uint16_t mab_tx_fifo_init_deassert_time)
{
    uint32_t reg_p2_sig_deassert_times_3=0;

#ifdef VALIDATE_PARMS
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_p2_sig_deassert_times_3 = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P2_SIG_DEASSERT_TIMES_3, MAB_TX_CREDIT_DISAB_DEASSERT_TIME, reg_p2_sig_deassert_times_3, mab_tx_credit_disab_deassert_time);
    reg_p2_sig_deassert_times_3 = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P2_SIG_DEASSERT_TIMES_3, MAB_TX_FIFO_INIT_DEASSERT_TIME, reg_p2_sig_deassert_times_3, mab_tx_fifo_init_deassert_time);

    RU_REG_WRITE(xlmac_id, XPORT_PORTRESET, P2_SIG_DEASSERT_TIMES_3, reg_p2_sig_deassert_times_3);

    return 0;
}

int ag_drv_xport_portreset_p2_sig_deassert_times_3_get(uint8_t xlmac_id, uint16_t *mab_tx_credit_disab_deassert_time, uint16_t *mab_tx_fifo_init_deassert_time)
{
    uint32_t reg_p2_sig_deassert_times_3=0;

#ifdef VALIDATE_PARMS
    if(!mab_tx_credit_disab_deassert_time || !mab_tx_fifo_init_deassert_time)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(xlmac_id, XPORT_PORTRESET, P2_SIG_DEASSERT_TIMES_3, reg_p2_sig_deassert_times_3);

    *mab_tx_credit_disab_deassert_time = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P2_SIG_DEASSERT_TIMES_3, MAB_TX_CREDIT_DISAB_DEASSERT_TIME, reg_p2_sig_deassert_times_3);
    *mab_tx_fifo_init_deassert_time = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P2_SIG_DEASSERT_TIMES_3, MAB_TX_FIFO_INIT_DEASSERT_TIME, reg_p2_sig_deassert_times_3);

    return 0;
}

int ag_drv_xport_portreset_p2_sig_deassert_times_4_set(uint8_t xlmac_id, uint16_t port_is_under_reset_deassert_time)
{
    uint32_t reg_p2_sig_deassert_times_4=0;

#ifdef VALIDATE_PARMS
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_p2_sig_deassert_times_4 = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P2_SIG_DEASSERT_TIMES_4, PORT_IS_UNDER_RESET_DEASSERT_TIME, reg_p2_sig_deassert_times_4, port_is_under_reset_deassert_time);

    RU_REG_WRITE(xlmac_id, XPORT_PORTRESET, P2_SIG_DEASSERT_TIMES_4, reg_p2_sig_deassert_times_4);

    return 0;
}

int ag_drv_xport_portreset_p2_sig_deassert_times_4_get(uint8_t xlmac_id, uint16_t *port_is_under_reset_deassert_time)
{
    uint32_t reg_p2_sig_deassert_times_4=0;

#ifdef VALIDATE_PARMS
    if(!port_is_under_reset_deassert_time)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(xlmac_id, XPORT_PORTRESET, P2_SIG_DEASSERT_TIMES_4, reg_p2_sig_deassert_times_4);

    *port_is_under_reset_deassert_time = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P2_SIG_DEASSERT_TIMES_4, PORT_IS_UNDER_RESET_DEASSERT_TIME, reg_p2_sig_deassert_times_4);

    return 0;
}

int ag_drv_xport_portreset_p3_sig_en_set(uint8_t xlmac_id, const xport_portreset_sig_en *p3_sig_en)
{
    uint32_t reg_p3_sig_en=0;

#ifdef VALIDATE_PARMS
    if(!p3_sig_en)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT) ||
       (p3_sig_en->enable_xlmac_rx_disab >= _1BITS_MAX_VAL_) ||
       (p3_sig_en->enable_xlmac_tx_disab >= _1BITS_MAX_VAL_) ||
       (p3_sig_en->enable_xlmac_tx_discard >= _1BITS_MAX_VAL_) ||
       (p3_sig_en->enable_xlmac_soft_reset >= _1BITS_MAX_VAL_) ||
       (p3_sig_en->enable_mab_rx_port_init >= _1BITS_MAX_VAL_) ||
       (p3_sig_en->enable_mab_tx_port_init >= _1BITS_MAX_VAL_) ||
       (p3_sig_en->enable_mab_tx_credit_disab >= _1BITS_MAX_VAL_) ||
       (p3_sig_en->enable_mab_tx_fifo_init >= _1BITS_MAX_VAL_) ||
       (p3_sig_en->enable_port_is_under_reset >= _1BITS_MAX_VAL_) ||
       (p3_sig_en->enable_xlmac_ep_discard >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_p3_sig_en = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P3_SIG_EN, ENABLE_XLMAC_RX_DISAB, reg_p3_sig_en, p3_sig_en->enable_xlmac_rx_disab);
    reg_p3_sig_en = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P3_SIG_EN, ENABLE_XLMAC_TX_DISAB, reg_p3_sig_en, p3_sig_en->enable_xlmac_tx_disab);
    reg_p3_sig_en = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P3_SIG_EN, ENABLE_XLMAC_TX_DISCARD, reg_p3_sig_en, p3_sig_en->enable_xlmac_tx_discard);
    reg_p3_sig_en = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P3_SIG_EN, ENABLE_XLMAC_SOFT_RESET, reg_p3_sig_en, p3_sig_en->enable_xlmac_soft_reset);
    reg_p3_sig_en = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P3_SIG_EN, ENABLE_MAB_RX_PORT_INIT, reg_p3_sig_en, p3_sig_en->enable_mab_rx_port_init);
    reg_p3_sig_en = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P3_SIG_EN, ENABLE_MAB_TX_PORT_INIT, reg_p3_sig_en, p3_sig_en->enable_mab_tx_port_init);
    reg_p3_sig_en = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P3_SIG_EN, ENABLE_MAB_TX_CREDIT_DISAB, reg_p3_sig_en, p3_sig_en->enable_mab_tx_credit_disab);
    reg_p3_sig_en = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P3_SIG_EN, ENABLE_MAB_TX_FIFO_INIT, reg_p3_sig_en, p3_sig_en->enable_mab_tx_fifo_init);
    reg_p3_sig_en = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P3_SIG_EN, ENABLE_PORT_IS_UNDER_RESET, reg_p3_sig_en, p3_sig_en->enable_port_is_under_reset);
    reg_p3_sig_en = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P3_SIG_EN, ENABLE_XLMAC_EP_DISCARD, reg_p3_sig_en, p3_sig_en->enable_xlmac_ep_discard);

    RU_REG_WRITE(xlmac_id, XPORT_PORTRESET, P3_SIG_EN, reg_p3_sig_en);

    return 0;
}

int ag_drv_xport_portreset_p3_sig_en_get(uint8_t xlmac_id, xport_portreset_sig_en *p3_sig_en)
{
    uint32_t reg_p3_sig_en=0;

#ifdef VALIDATE_PARMS
    if(!p3_sig_en)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(xlmac_id, XPORT_PORTRESET, P3_SIG_EN, reg_p3_sig_en);

    p3_sig_en->enable_xlmac_rx_disab = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P3_SIG_EN, ENABLE_XLMAC_RX_DISAB, reg_p3_sig_en);
    p3_sig_en->enable_xlmac_tx_disab = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P3_SIG_EN, ENABLE_XLMAC_TX_DISAB, reg_p3_sig_en);
    p3_sig_en->enable_xlmac_tx_discard = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P3_SIG_EN, ENABLE_XLMAC_TX_DISCARD, reg_p3_sig_en);
    p3_sig_en->enable_xlmac_soft_reset = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P3_SIG_EN, ENABLE_XLMAC_SOFT_RESET, reg_p3_sig_en);
    p3_sig_en->enable_mab_rx_port_init = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P3_SIG_EN, ENABLE_MAB_RX_PORT_INIT, reg_p3_sig_en);
    p3_sig_en->enable_mab_tx_port_init = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P3_SIG_EN, ENABLE_MAB_TX_PORT_INIT, reg_p3_sig_en);
    p3_sig_en->enable_mab_tx_credit_disab = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P3_SIG_EN, ENABLE_MAB_TX_CREDIT_DISAB, reg_p3_sig_en);
    p3_sig_en->enable_mab_tx_fifo_init = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P3_SIG_EN, ENABLE_MAB_TX_FIFO_INIT, reg_p3_sig_en);
    p3_sig_en->enable_port_is_under_reset = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P3_SIG_EN, ENABLE_PORT_IS_UNDER_RESET, reg_p3_sig_en);
    p3_sig_en->enable_xlmac_ep_discard = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P3_SIG_EN, ENABLE_XLMAC_EP_DISCARD, reg_p3_sig_en);

    return 0;
}

int ag_drv_xport_portreset_p3_sig_assert_times_0_set(uint8_t xlmac_id, uint16_t xlmac_rx_disab_assert_time, uint16_t xlmac_tx_disab_assert_time)
{
    uint32_t reg_p3_sig_assert_times_0=0;

#ifdef VALIDATE_PARMS
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_p3_sig_assert_times_0 = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P3_SIG_ASSERT_TIMES_0, XLMAC_RX_DISAB_ASSERT_TIME, reg_p3_sig_assert_times_0, xlmac_rx_disab_assert_time);
    reg_p3_sig_assert_times_0 = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P3_SIG_ASSERT_TIMES_0, XLMAC_TX_DISAB_ASSERT_TIME, reg_p3_sig_assert_times_0, xlmac_tx_disab_assert_time);

    RU_REG_WRITE(xlmac_id, XPORT_PORTRESET, P3_SIG_ASSERT_TIMES_0, reg_p3_sig_assert_times_0);

    return 0;
}

int ag_drv_xport_portreset_p3_sig_assert_times_0_get(uint8_t xlmac_id, uint16_t *xlmac_rx_disab_assert_time, uint16_t *xlmac_tx_disab_assert_time)
{
    uint32_t reg_p3_sig_assert_times_0=0;

#ifdef VALIDATE_PARMS
    if(!xlmac_rx_disab_assert_time || !xlmac_tx_disab_assert_time)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(xlmac_id, XPORT_PORTRESET, P3_SIG_ASSERT_TIMES_0, reg_p3_sig_assert_times_0);

    *xlmac_rx_disab_assert_time = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P3_SIG_ASSERT_TIMES_0, XLMAC_RX_DISAB_ASSERT_TIME, reg_p3_sig_assert_times_0);
    *xlmac_tx_disab_assert_time = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P3_SIG_ASSERT_TIMES_0, XLMAC_TX_DISAB_ASSERT_TIME, reg_p3_sig_assert_times_0);

    return 0;
}

int ag_drv_xport_portreset_p3_sig_assert_times_1_set(uint8_t xlmac_id, uint16_t xlmac_txdiscard_assert_time, uint16_t xlmac_soft_reset_assert_time)
{
    uint32_t reg_p3_sig_assert_times_1=0;

#ifdef VALIDATE_PARMS
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_p3_sig_assert_times_1 = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P3_SIG_ASSERT_TIMES_1, XLMAC_TXDISCARD_ASSERT_TIME, reg_p3_sig_assert_times_1, xlmac_txdiscard_assert_time);
    reg_p3_sig_assert_times_1 = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P3_SIG_ASSERT_TIMES_1, XLMAC_SOFT_RESET_ASSERT_TIME, reg_p3_sig_assert_times_1, xlmac_soft_reset_assert_time);

    RU_REG_WRITE(xlmac_id, XPORT_PORTRESET, P3_SIG_ASSERT_TIMES_1, reg_p3_sig_assert_times_1);

    return 0;
}

int ag_drv_xport_portreset_p3_sig_assert_times_1_get(uint8_t xlmac_id, uint16_t *xlmac_txdiscard_assert_time, uint16_t *xlmac_soft_reset_assert_time)
{
    uint32_t reg_p3_sig_assert_times_1=0;

#ifdef VALIDATE_PARMS
    if(!xlmac_txdiscard_assert_time || !xlmac_soft_reset_assert_time)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(xlmac_id, XPORT_PORTRESET, P3_SIG_ASSERT_TIMES_1, reg_p3_sig_assert_times_1);

    *xlmac_txdiscard_assert_time = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P3_SIG_ASSERT_TIMES_1, XLMAC_TXDISCARD_ASSERT_TIME, reg_p3_sig_assert_times_1);
    *xlmac_soft_reset_assert_time = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P3_SIG_ASSERT_TIMES_1, XLMAC_SOFT_RESET_ASSERT_TIME, reg_p3_sig_assert_times_1);

    return 0;
}

int ag_drv_xport_portreset_p3_sig_assert_times_2_set(uint8_t xlmac_id, uint16_t mab_rx_port_init_assert_time, uint16_t mab_tx_port_init_assert_time)
{
    uint32_t reg_p3_sig_assert_times_2=0;

#ifdef VALIDATE_PARMS
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_p3_sig_assert_times_2 = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P3_SIG_ASSERT_TIMES_2, MAB_RX_PORT_INIT_ASSERT_TIME, reg_p3_sig_assert_times_2, mab_rx_port_init_assert_time);
    reg_p3_sig_assert_times_2 = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P3_SIG_ASSERT_TIMES_2, MAB_TX_PORT_INIT_ASSERT_TIME, reg_p3_sig_assert_times_2, mab_tx_port_init_assert_time);

    RU_REG_WRITE(xlmac_id, XPORT_PORTRESET, P3_SIG_ASSERT_TIMES_2, reg_p3_sig_assert_times_2);

    return 0;
}

int ag_drv_xport_portreset_p3_sig_assert_times_2_get(uint8_t xlmac_id, uint16_t *mab_rx_port_init_assert_time, uint16_t *mab_tx_port_init_assert_time)
{
    uint32_t reg_p3_sig_assert_times_2=0;

#ifdef VALIDATE_PARMS
    if(!mab_rx_port_init_assert_time || !mab_tx_port_init_assert_time)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(xlmac_id, XPORT_PORTRESET, P3_SIG_ASSERT_TIMES_2, reg_p3_sig_assert_times_2);

    *mab_rx_port_init_assert_time = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P3_SIG_ASSERT_TIMES_2, MAB_RX_PORT_INIT_ASSERT_TIME, reg_p3_sig_assert_times_2);
    *mab_tx_port_init_assert_time = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P3_SIG_ASSERT_TIMES_2, MAB_TX_PORT_INIT_ASSERT_TIME, reg_p3_sig_assert_times_2);

    return 0;
}

int ag_drv_xport_portreset_p3_sig_assert_times_3_set(uint8_t xlmac_id, uint16_t mab_tx_credit_disab_assert_time, uint16_t mab_tx_fifo_init_assert_time)
{
    uint32_t reg_p3_sig_assert_times_3=0;

#ifdef VALIDATE_PARMS
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_p3_sig_assert_times_3 = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P3_SIG_ASSERT_TIMES_3, MAB_TX_CREDIT_DISAB_ASSERT_TIME, reg_p3_sig_assert_times_3, mab_tx_credit_disab_assert_time);
    reg_p3_sig_assert_times_3 = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P3_SIG_ASSERT_TIMES_3, MAB_TX_FIFO_INIT_ASSERT_TIME, reg_p3_sig_assert_times_3, mab_tx_fifo_init_assert_time);

    RU_REG_WRITE(xlmac_id, XPORT_PORTRESET, P3_SIG_ASSERT_TIMES_3, reg_p3_sig_assert_times_3);

    return 0;
}

int ag_drv_xport_portreset_p3_sig_assert_times_3_get(uint8_t xlmac_id, uint16_t *mab_tx_credit_disab_assert_time, uint16_t *mab_tx_fifo_init_assert_time)
{
    uint32_t reg_p3_sig_assert_times_3=0;

#ifdef VALIDATE_PARMS
    if(!mab_tx_credit_disab_assert_time || !mab_tx_fifo_init_assert_time)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(xlmac_id, XPORT_PORTRESET, P3_SIG_ASSERT_TIMES_3, reg_p3_sig_assert_times_3);

    *mab_tx_credit_disab_assert_time = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P3_SIG_ASSERT_TIMES_3, MAB_TX_CREDIT_DISAB_ASSERT_TIME, reg_p3_sig_assert_times_3);
    *mab_tx_fifo_init_assert_time = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P3_SIG_ASSERT_TIMES_3, MAB_TX_FIFO_INIT_ASSERT_TIME, reg_p3_sig_assert_times_3);

    return 0;
}

int ag_drv_xport_portreset_p3_sig_assert_times_4_set(uint8_t xlmac_id, uint16_t port_is_under_reset_assert_time)
{
    uint32_t reg_p3_sig_assert_times_4=0;

#ifdef VALIDATE_PARMS
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_p3_sig_assert_times_4 = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P3_SIG_ASSERT_TIMES_4, PORT_IS_UNDER_RESET_ASSERT_TIME, reg_p3_sig_assert_times_4, port_is_under_reset_assert_time);

    RU_REG_WRITE(xlmac_id, XPORT_PORTRESET, P3_SIG_ASSERT_TIMES_4, reg_p3_sig_assert_times_4);

    return 0;
}

int ag_drv_xport_portreset_p3_sig_assert_times_4_get(uint8_t xlmac_id, uint16_t *port_is_under_reset_assert_time)
{
    uint32_t reg_p3_sig_assert_times_4=0;

#ifdef VALIDATE_PARMS
    if(!port_is_under_reset_assert_time)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(xlmac_id, XPORT_PORTRESET, P3_SIG_ASSERT_TIMES_4, reg_p3_sig_assert_times_4);

    *port_is_under_reset_assert_time = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P3_SIG_ASSERT_TIMES_4, PORT_IS_UNDER_RESET_ASSERT_TIME, reg_p3_sig_assert_times_4);

    return 0;
}

int ag_drv_xport_portreset_p3_sig_deassert_times_0_set(uint8_t xlmac_id, uint16_t xlmac_rx_disab_deassert_time, uint16_t xlmac_tx_disab_deassert_time)
{
    uint32_t reg_p3_sig_deassert_times_0=0;

#ifdef VALIDATE_PARMS
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_p3_sig_deassert_times_0 = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P3_SIG_DEASSERT_TIMES_0, XLMAC_RX_DISAB_DEASSERT_TIME, reg_p3_sig_deassert_times_0, xlmac_rx_disab_deassert_time);
    reg_p3_sig_deassert_times_0 = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P3_SIG_DEASSERT_TIMES_0, XLMAC_TX_DISAB_DEASSERT_TIME, reg_p3_sig_deassert_times_0, xlmac_tx_disab_deassert_time);

    RU_REG_WRITE(xlmac_id, XPORT_PORTRESET, P3_SIG_DEASSERT_TIMES_0, reg_p3_sig_deassert_times_0);

    return 0;
}

int ag_drv_xport_portreset_p3_sig_deassert_times_0_get(uint8_t xlmac_id, uint16_t *xlmac_rx_disab_deassert_time, uint16_t *xlmac_tx_disab_deassert_time)
{
    uint32_t reg_p3_sig_deassert_times_0=0;

#ifdef VALIDATE_PARMS
    if(!xlmac_rx_disab_deassert_time || !xlmac_tx_disab_deassert_time)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(xlmac_id, XPORT_PORTRESET, P3_SIG_DEASSERT_TIMES_0, reg_p3_sig_deassert_times_0);

    *xlmac_rx_disab_deassert_time = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P3_SIG_DEASSERT_TIMES_0, XLMAC_RX_DISAB_DEASSERT_TIME, reg_p3_sig_deassert_times_0);
    *xlmac_tx_disab_deassert_time = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P3_SIG_DEASSERT_TIMES_0, XLMAC_TX_DISAB_DEASSERT_TIME, reg_p3_sig_deassert_times_0);

    return 0;
}

int ag_drv_xport_portreset_p3_sig_deassert_times_1_set(uint8_t xlmac_id, uint16_t xlmac_txdiscard_deassert_time, uint16_t xlmac_soft_reset_deassert_time)
{
    uint32_t reg_p3_sig_deassert_times_1=0;

#ifdef VALIDATE_PARMS
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_p3_sig_deassert_times_1 = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P3_SIG_DEASSERT_TIMES_1, XLMAC_TXDISCARD_DEASSERT_TIME, reg_p3_sig_deassert_times_1, xlmac_txdiscard_deassert_time);
    reg_p3_sig_deassert_times_1 = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P3_SIG_DEASSERT_TIMES_1, XLMAC_SOFT_RESET_DEASSERT_TIME, reg_p3_sig_deassert_times_1, xlmac_soft_reset_deassert_time);

    RU_REG_WRITE(xlmac_id, XPORT_PORTRESET, P3_SIG_DEASSERT_TIMES_1, reg_p3_sig_deassert_times_1);

    return 0;
}

int ag_drv_xport_portreset_p3_sig_deassert_times_1_get(uint8_t xlmac_id, uint16_t *xlmac_txdiscard_deassert_time, uint16_t *xlmac_soft_reset_deassert_time)
{
    uint32_t reg_p3_sig_deassert_times_1=0;

#ifdef VALIDATE_PARMS
    if(!xlmac_txdiscard_deassert_time || !xlmac_soft_reset_deassert_time)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(xlmac_id, XPORT_PORTRESET, P3_SIG_DEASSERT_TIMES_1, reg_p3_sig_deassert_times_1);

    *xlmac_txdiscard_deassert_time = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P3_SIG_DEASSERT_TIMES_1, XLMAC_TXDISCARD_DEASSERT_TIME, reg_p3_sig_deassert_times_1);
    *xlmac_soft_reset_deassert_time = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P3_SIG_DEASSERT_TIMES_1, XLMAC_SOFT_RESET_DEASSERT_TIME, reg_p3_sig_deassert_times_1);

    return 0;
}

int ag_drv_xport_portreset_p3_sig_deassert_times_2_set(uint8_t xlmac_id, uint16_t mab_rx_port_init_deassert_time, uint16_t mab_tx_port_init_deassert_time)
{
    uint32_t reg_p3_sig_deassert_times_2=0;

#ifdef VALIDATE_PARMS
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_p3_sig_deassert_times_2 = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P3_SIG_DEASSERT_TIMES_2, MAB_RX_PORT_INIT_DEASSERT_TIME, reg_p3_sig_deassert_times_2, mab_rx_port_init_deassert_time);
    reg_p3_sig_deassert_times_2 = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P3_SIG_DEASSERT_TIMES_2, MAB_TX_PORT_INIT_DEASSERT_TIME, reg_p3_sig_deassert_times_2, mab_tx_port_init_deassert_time);

    RU_REG_WRITE(xlmac_id, XPORT_PORTRESET, P3_SIG_DEASSERT_TIMES_2, reg_p3_sig_deassert_times_2);

    return 0;
}

int ag_drv_xport_portreset_p3_sig_deassert_times_2_get(uint8_t xlmac_id, uint16_t *mab_rx_port_init_deassert_time, uint16_t *mab_tx_port_init_deassert_time)
{
    uint32_t reg_p3_sig_deassert_times_2=0;

#ifdef VALIDATE_PARMS
    if(!mab_rx_port_init_deassert_time || !mab_tx_port_init_deassert_time)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(xlmac_id, XPORT_PORTRESET, P3_SIG_DEASSERT_TIMES_2, reg_p3_sig_deassert_times_2);

    *mab_rx_port_init_deassert_time = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P3_SIG_DEASSERT_TIMES_2, MAB_RX_PORT_INIT_DEASSERT_TIME, reg_p3_sig_deassert_times_2);
    *mab_tx_port_init_deassert_time = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P3_SIG_DEASSERT_TIMES_2, MAB_TX_PORT_INIT_DEASSERT_TIME, reg_p3_sig_deassert_times_2);

    return 0;
}

int ag_drv_xport_portreset_p3_sig_deassert_times_3_set(uint8_t xlmac_id, uint16_t mab_tx_credit_disab_deassert_time, uint16_t mab_tx_fifo_init_deassert_time)
{
    uint32_t reg_p3_sig_deassert_times_3=0;

#ifdef VALIDATE_PARMS
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_p3_sig_deassert_times_3 = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P3_SIG_DEASSERT_TIMES_3, MAB_TX_CREDIT_DISAB_DEASSERT_TIME, reg_p3_sig_deassert_times_3, mab_tx_credit_disab_deassert_time);
    reg_p3_sig_deassert_times_3 = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P3_SIG_DEASSERT_TIMES_3, MAB_TX_FIFO_INIT_DEASSERT_TIME, reg_p3_sig_deassert_times_3, mab_tx_fifo_init_deassert_time);

    RU_REG_WRITE(xlmac_id, XPORT_PORTRESET, P3_SIG_DEASSERT_TIMES_3, reg_p3_sig_deassert_times_3);

    return 0;
}

int ag_drv_xport_portreset_p3_sig_deassert_times_3_get(uint8_t xlmac_id, uint16_t *mab_tx_credit_disab_deassert_time, uint16_t *mab_tx_fifo_init_deassert_time)
{
    uint32_t reg_p3_sig_deassert_times_3=0;

#ifdef VALIDATE_PARMS
    if(!mab_tx_credit_disab_deassert_time || !mab_tx_fifo_init_deassert_time)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(xlmac_id, XPORT_PORTRESET, P3_SIG_DEASSERT_TIMES_3, reg_p3_sig_deassert_times_3);

    *mab_tx_credit_disab_deassert_time = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P3_SIG_DEASSERT_TIMES_3, MAB_TX_CREDIT_DISAB_DEASSERT_TIME, reg_p3_sig_deassert_times_3);
    *mab_tx_fifo_init_deassert_time = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P3_SIG_DEASSERT_TIMES_3, MAB_TX_FIFO_INIT_DEASSERT_TIME, reg_p3_sig_deassert_times_3);

    return 0;
}

int ag_drv_xport_portreset_p3_sig_deassert_times_4_set(uint8_t xlmac_id, uint16_t port_is_under_reset_deassert_time)
{
    uint32_t reg_p3_sig_deassert_times_4=0;

#ifdef VALIDATE_PARMS
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_p3_sig_deassert_times_4 = RU_FIELD_SET(xlmac_id, XPORT_PORTRESET, P3_SIG_DEASSERT_TIMES_4, PORT_IS_UNDER_RESET_DEASSERT_TIME, reg_p3_sig_deassert_times_4, port_is_under_reset_deassert_time);

    RU_REG_WRITE(xlmac_id, XPORT_PORTRESET, P3_SIG_DEASSERT_TIMES_4, reg_p3_sig_deassert_times_4);

    return 0;
}

int ag_drv_xport_portreset_p3_sig_deassert_times_4_get(uint8_t xlmac_id, uint16_t *port_is_under_reset_deassert_time)
{
    uint32_t reg_p3_sig_deassert_times_4=0;

#ifdef VALIDATE_PARMS
    if(!port_is_under_reset_deassert_time)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(xlmac_id, XPORT_PORTRESET, P3_SIG_DEASSERT_TIMES_4, reg_p3_sig_deassert_times_4);

    *port_is_under_reset_deassert_time = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, P3_SIG_DEASSERT_TIMES_4, PORT_IS_UNDER_RESET_DEASSERT_TIME, reg_p3_sig_deassert_times_4);

    return 0;
}

int ag_drv_xport_portreset_debug_get(uint8_t xlmac_id, uint8_t *p3_sm_state, uint8_t *p2_sm_state, uint8_t *p1_sm_state, uint8_t *p0_sm_state)
{
    uint32_t reg_debug=0;

#ifdef VALIDATE_PARMS
    if(!p3_sm_state || !p2_sm_state || !p1_sm_state || !p0_sm_state)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
    if((xlmac_id >= BLOCK_ADDR_COUNT))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    RU_REG_READ(xlmac_id, XPORT_PORTRESET, DEBUG, reg_debug);

    *p3_sm_state = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, DEBUG, P3_SM_STATE, reg_debug);
    *p2_sm_state = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, DEBUG, P2_SM_STATE, reg_debug);
    *p1_sm_state = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, DEBUG, P1_SM_STATE, reg_debug);
    *p0_sm_state = RU_FIELD_GET(xlmac_id, XPORT_PORTRESET, DEBUG, P0_SM_STATE, reg_debug);

    return 0;
}

