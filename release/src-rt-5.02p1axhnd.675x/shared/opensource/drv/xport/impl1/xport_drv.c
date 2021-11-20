/*
   Copyright (c) 2017 Broadcom Corporation
   All Rights Reserved

   <:label-BRCM:2017:DUAL/GPL:standard

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
/*
 * xport_drv.c
 *
 */

//includes
#include "xport_ag.h"
#include "xport_drv.h"
#include <asm/delay.h>
#include "xport_intr.h"
#include "bcm_map_part.h"
void remap_ru_block_addrs(uint32_t block_index, const ru_block_rec *ru_blocks[]);
#define UDELAY udelay
#include "mii_shared.h"
#include "boardparms.h"

static int validate_xport_configuration(xport_xlmac_port_info_s *init_params)
{
    if (XPORT_INTF_VALID(init_params->intf_type))
    {
        return XPORT_ERR_OK;
    }
    return XPORT_ERR_INVALID;
}

static int xport_speed_to_xlmac(XPORT_PORT_RATE xport_speed)
{
    int ret = 0;

    switch (xport_speed)
    {
    case XPORT_RATE_10MB:
        ret = XLMAC_PORT_SPEED_10MB;
        break;
    case XPORT_RATE_100MB:
        ret = XLMAC_PORT_SPEED_100MB;
        break;
    case XPORT_RATE_1000MB:
        ret = XLMAC_PORT_SPEED_1000MB;
        break;
    case XPORT_RATE_2500MB:
        ret = XLMAC_PORT_SPEED_2500MB;
        break;
    case XPORT_RATE_10G:
        ret = XLMAC_PORT_SPEED_10G;
        break;
    case XPORT_RATE_UNKNOWN:
    default:
        __xportError("Wrong xport speed %d\n", xport_speed);
    }

    return ret;
}

static XPORT_PORT_RATE xport_speed_get(uint8_t portid)
{
    uint8_t speed_mode, no_sop_for_crc_hg, hdr_mode;
    XPORT_PORT_RATE xport_speed = XPORT_RATE_UNKNOWN;
    
    if (ag_drv_xport_xlmac_core_mode_get(portid, &speed_mode, &no_sop_for_crc_hg, &hdr_mode))
        return XPORT_RATE_UNKNOWN;
    
    switch (speed_mode)
    {
    case XLMAC_PORT_SPEED_10MB: /* 10M */
        xport_speed = XPORT_RATE_10MB;
        break;

    case XLMAC_PORT_SPEED_100MB: /* 100Mbps */
        xport_speed = XPORT_RATE_100MB;
        break;

    case XLMAC_PORT_SPEED_1000MB: /* 1000Mbps */
        xport_speed = XPORT_RATE_1000MB;
        break;

    case XLMAC_PORT_SPEED_2500MB: /* 2500Mbps */
        xport_speed = XPORT_RATE_2500MB;
        break;

    case XLMAC_PORT_SPEED_10G: /* 10G */
        xport_speed = XPORT_RATE_10G;
        break;

    default:
        xport_speed = XPORT_RATE_UNKNOWN;
    }
    
    return xport_speed;
}

static int xport_msbus_reset(xport_xlmac_port_info_s *init_params)
{
    xport_mab_ctrl ctrl_xlmac;
    int rc = XPORT_ERR_OK;
    XPORT_PORT_ID xlmac_port = init_params->xport_port_id;

    rc = rc ? rc : ag_drv_xport_mab_ctrl_get(&ctrl_xlmac);

    if (xlmac_port == XPORT_PORT_ID_AE)
    {
        ctrl_xlmac.xgmii_rx_rst = 1;
        ctrl_xlmac.xgmii_tx_rst = 1;
    }

    ctrl_xlmac.gmii_rx_rst |= (1 << xlmac_port);
    ctrl_xlmac.gmii_tx_rst |= (1 << xlmac_port);

    rc = rc ? rc : ag_drv_xport_mab_ctrl_set(&ctrl_xlmac);

    __xportDebug("DONE (%d)\n", rc);

    return rc;
}

static int xport_xlmac_reset(xport_xlmac_port_info_s *init_params)
{
    xport_xlmac_core_ctrl ctrl;
    int rc = XPORT_ERR_INVALID;
    if (XPORT_INTF_VALID(init_params->intf_type))
    {
        rc = ag_drv_xport_xlmac_core_ctrl_get(init_params->xport_port_id, &ctrl);
        ctrl.soft_reset = 1; /* Keep the XLMAC Port in soft reset */
        rc = rc ? rc : ag_drv_xport_xlmac_core_ctrl_set(init_params->xport_port_id, &ctrl);
    }
    __xportDebug("DONE (%d)\n", rc);

    return rc;
}

static int xlif_reset_credit(int channel)
{
    volatile XlifReg *p_xlif_ch = XLIF_REG;
    p_xlif_ch[channel].xlif_tx_if_str.set_credits = (1<<12);
    __xportDebug("DONE (channel = %d)\n", channel);
    return 0;
}

static int xport_xlif_reset(xport_xlmac_port_info_s *init_params)
{
    int rc = XPORT_ERR_INVALID;
    if (XPORT_INTF_VALID(init_params->intf_type))
    {
        rc = xlif_reset_credit(init_params->xport_port_id);
    }
    __xportDebug("DONE (%d)\n", rc);
    return rc;
}

static int xlif_release_credit(int channel)
{
    volatile XlifReg *p_xlif_ch = XLIF_REG;
    p_xlif_ch[channel].xlif_tx_if_str.set_credits &= ~(1<<12);
    __xportDebug("DONE (channel = %d)\n", channel);
    return 0;
}

static int xport_xlif_release(xport_xlmac_port_info_s *init_params)
{
    int rc = XPORT_ERR_INVALID;
    if (XPORT_INTF_VALID(init_params->intf_type))
    {
        rc = xlif_release_credit(init_params->xport_port_id);
    }
    __xportDebug("DONE (%d)\n", rc);
    return rc;
}

static int xport_xlmac_release_reset(xport_xlmac_port_info_s *init_params)
{
    xport_xlmac_core_ctrl ctrl;
    int rc = XPORT_ERR_INVALID;
    if (XPORT_INTF_VALID(init_params->intf_type))
    {
        rc = ag_drv_xport_xlmac_core_ctrl_get(init_params->xport_port_id, &ctrl);
        ctrl.soft_reset = 0; /* Release the XLMAC Port soft reset */
        rc = rc ? rc : ag_drv_xport_xlmac_core_ctrl_set(init_params->xport_port_id, &ctrl);
    }
    __xportDebug("DONE (rc = %d)\n", rc);
    return rc;
}

static int xport_xlmac_init(xport_xlmac_port_info_s *init_params)
{
    xport_xlmac_core_ctrl     ctrl;
    xport_xlmac_core_tx_ctrl  tx_ctrl;
    xport_xlmac_core_rx_ctrl  rx_ctrl;
    xport_xlmac_core_pfc_ctrl pfc_ctrl;

    uint32_t port_id;
    int rc = XPORT_ERR_OK;
    int p0_xgmii = 0;

    /* XPORT-XLMAC should be in reset during init */

    if (XPORT_INTF_VALID(init_params->intf_type))
    {
        /* Set P0 in XGMII if speed is 10G */
        if (init_params->intf_type == XPORT_INTF_AE )
        {
            if (init_params->port_rate == XPORT_RATE_10G) p0_xgmii = 1;
        }
    }
    else
    {
        return XPORT_ERR_INVALID;
    }

    /* Only initialize the port in use */
    port_id = init_params->xport_port_id;

    /* Enable 2.5G/10G AE PFC_STATS_EN for Hardware work around */
    rc = rc? rc: ag_drv_xport_xlmac_core_pfc_ctrl_get(port_id, &pfc_ctrl);
    pfc_ctrl.pfc_stats_en = 1;  /* Work around for HW issue */
    rc = rc? rc: ag_drv_xport_xlmac_core_pfc_ctrl_set(port_id, &pfc_ctrl);

    /* XPORT_REG_XPORT_CNTRL_1 */
    rc = rc ? rc : ag_drv_xport_reg_xport_cntrl_1_set(0 /* msbus_clk_sel - never operate both link simulteneously*/,
                                                      0 /* wan_led0_sel - keep default. Need update if design share LED for both wan ports*/, 
                                                      0 /* timeout_rst_disable */, 
                                                      p0_xgmii?1:0 /* p0_mode; 0=GMII, 1=XGMII */);


    rc = rc ? rc : ag_drv_xport_xlmac_core_tx_ctrl_get(port_id, &tx_ctrl);

    tx_ctrl.crc_mode = XLMAC_TX_CTRL_CRC_MODE_PER_PKT /* = 3 (not defined in RDB) */;
    tx_ctrl.pad_en = 1;
    tx_ctrl.tx_threshold = 2; /* TX_THRESHOLD depends on how "fast" XRDP writes into XPORT. 
                                 If any underflow in XLMAC we can increase the value. */
    rc = rc ? rc : ag_drv_xport_xlmac_core_tx_ctrl_set(port_id, &tx_ctrl);


    rc = rc ? rc : ag_drv_xport_xlmac_core_rx_ctrl_get(port_id, &rx_ctrl);
    rx_ctrl.rx_any_start = 0;
    rx_ctrl.strip_crc = 0; /* CRC will be validated by the BBH */
    rx_ctrl.strict_preamble = 0; /* Keep this disabled otherwise interop issue with some bad equipment */
    rx_ctrl.runt_threshold = 0x40;
    rx_ctrl.rx_pass_ctrl = 1;
    rx_ctrl.rx_pass_pause = 0; /* By default do not pass RX pause to XRDP */
    rx_ctrl.rx_pass_pfc = 0;   /* By default do not pass RX PFC to XRDP */
    rc = rc ? rc : ag_drv_xport_xlmac_core_rx_ctrl_set(port_id, &rx_ctrl);

    rc = ag_drv_xport_xlmac_core_ctrl_get(port_id, &ctrl);
    ctrl.extended_hig2_en = 0; /* By default - do not enable hig2 */
    rc = rc ? rc : ag_drv_xport_xlmac_core_ctrl_set(port_id, &ctrl);
    {
        uint8_t speed_mode, no_sop_for_crc_hg, hdr_mode;
        rc = rc ? rc : ag_drv_xport_xlmac_core_mode_get(port_id, &speed_mode, &no_sop_for_crc_hg, &hdr_mode);
        no_sop_for_crc_hg = 0;
        hdr_mode = 0;
        /* Configure the SPEED_MODE in XLMAC_MODE register based on link speed detected */
        speed_mode = xport_speed_to_xlmac(init_params->port_rate);
        rc = rc ? rc : ag_drv_xport_xlmac_core_mode_set(port_id, speed_mode, no_sop_for_crc_hg, hdr_mode);
    }
    /* Release XPORT reset  */
    xport_xlmac_release_reset(init_params);

    __xportDebug("rc = %d; intf = %d port = %d spd = %s dup = %d\n",
                  rc,init_params->intf_type, init_params->xport_port_id,
                  xport_rate_to_str(init_params->port_rate),init_params->port_duplex);
    return rc;
}

static int xport_msbus_init(xport_xlmac_port_info_s *init_params)
{
    xport_mab_ctrl ctrl_xlmac;
    xport_mab_tx_wrr_ctrl tx_wrr_ctrl;
    XPORT_PORT_ID xlmac_port = init_params->xport_port_id;
    int rc = XPORT_ERR_OK;

    /* Keep MSBUS TX arbitrater ARB_Mode to HW defaults
     * Set unused port weight to ZERO */
    rc = rc ? rc : ag_drv_xport_mab_tx_wrr_ctrl_get(&tx_wrr_ctrl);
    tx_wrr_ctrl.arb_mode  = 1; /* Work-Conserving Mode */
    tx_wrr_ctrl.p2_weight = 0;
    tx_wrr_ctrl.p3_weight = 0;
    rc = rc ? rc : ag_drv_xport_mab_tx_wrr_ctrl_set(&tx_wrr_ctrl);


    rc = rc ? rc : ag_drv_xport_mab_ctrl_get(&ctrl_xlmac);
    if (xlmac_port == XPORT_PORT_ID_AE)
    {
        ctrl_xlmac.xgmii_rx_rst = 0;
        ctrl_xlmac.xgmii_tx_rst = 0;
    }

    ctrl_xlmac.gmii_rx_rst &= ~(1 << xlmac_port);
    ctrl_xlmac.gmii_tx_rst &= ~(1 << xlmac_port);

    rc = rc ? rc : ag_drv_xport_mab_ctrl_set(&ctrl_xlmac);

    __xportDebug("DONE (rc = %d)\n", rc);
    return rc;
}

static int xport_reset(xport_xlmac_port_info_s *init_params)
{
    int rc;
    rc = xport_msbus_reset(init_params);
    rc = rc ? rc : xport_xlmac_reset(init_params);
    rc = rc ? rc : xport_xlif_reset(init_params);
    __xportNotice("rc = %d; intf = %d port = %d spd = %s dup = %d\n",
                  rc,init_params->intf_type, init_params->xport_port_id,
                  xport_rate_to_str(init_params->port_rate),init_params->port_duplex);
    return rc;
}

static int xport_init(xport_xlmac_port_info_s *init_params)
{
    int rc;
    rc = xport_xlif_release(init_params);
    rc = rc ? rc : xport_xlmac_init(init_params);
    rc = rc ? rc : xport_msbus_init(init_params);
    __xportNotice("rc = %d; intf = %d port = %d spd = %s dup = %d\n",
                  rc,init_params->intf_type, init_params->xport_port_id,
                  xport_rate_to_str(init_params->port_rate),init_params->port_duplex);
    return rc;
}

int xport_init_driver(xport_xlmac_port_info_s *init_params)
{
    /* MUST only be called once */
    static int remap_done = 0;
    if (!remap_done) 
    {
        /* ioRemap virtual addresses of XPORT */
        remap_ru_block_addrs(XPORT_IDX, RU_XPORT_BLOCKS);
        remap_done = 1;
    }

    if (validate_xport_configuration(init_params))
    {
        pr_err("XPORT configuration validation failed\n");
        return XPORT_ERR_PARAM;
    }

    /* Serdes insert/remove, PHY/RGMII link up/down interrupts could be registered here */

    if (xport_reset(init_params))
        return XPORT_ERR_PARAM;

    __xportDebug("DONE \n");
    return XPORT_ERR_OK;
}

int xport_handle_link_up(xport_xlmac_port_info_s *p_info)
{
    int rc = 0;

    rc = rc ? rc : xport_init(p_info);
    __xportDebug("DONE (rc = %d)\n", rc);
    return rc;
}

int xport_handle_link_dn(xport_xlmac_port_info_s *p_info)
{
    int rc = 0;
    rc = rc ? rc : xport_reset(p_info);
    UDELAY(1000);
    __xportDebug("DONE (rc = %d)\n", rc);
    //return xport_reset(&local_xport_cfg);
    return rc;
}
int xport_get_port_configuration(uint32_t portid, xport_port_cfg_s *port_conf)
{
    int rc;
    xport_xlmac_core_ctrl xlmac_ctrl;
    xport_xlmac_core_tx_ctrl xlmac_tx_ctrl;
    xport_xlmac_core_rx_ctrl xlmac_rx_ctrl;

    rc = ag_drv_xport_xlmac_core_ctrl_get(portid, &xlmac_ctrl);
    rc = rc ? rc : ag_drv_xport_xlmac_core_tx_ctrl_get(portid, &xlmac_tx_ctrl);
    rc = rc ? rc : ag_drv_xport_xlmac_core_rx_ctrl_get(portid, &xlmac_rx_ctrl);
    port_conf->average_igp = xlmac_tx_ctrl.average_ipg;
    port_conf->pad_en = xlmac_tx_ctrl.pad_en;
    port_conf->local_loopback = xlmac_ctrl.local_lpbk;
    port_conf->pad_threashold = xlmac_tx_ctrl.pad_threshold;
    port_conf->tx_preamble_len = xlmac_tx_ctrl.tx_preamble_length;
    port_conf->tx_threshold = xlmac_tx_ctrl.tx_threshold;
    port_conf->speed = xport_speed_get((uint8_t)portid);
    __xportDebug("DONE \n");
    return rc;
}

int xport_set_port_configuration(uint32_t portid, xport_port_cfg_s *port_conf)
{
    int rc = 0;
    xport_xlmac_core_ctrl xlmac_ctrl;
    xport_xlmac_core_tx_ctrl xlmac_tx_ctrl;

    rc = rc ? rc : ag_drv_xport_xlmac_core_ctrl_get(portid, &xlmac_ctrl);
    rc = rc ? rc : ag_drv_xport_xlmac_core_tx_ctrl_get(portid, &xlmac_tx_ctrl);

    xlmac_tx_ctrl.average_ipg = port_conf->average_igp;
    xlmac_tx_ctrl.pad_en = port_conf->pad_en;
    xlmac_ctrl.local_lpbk = port_conf->local_loopback;
    xlmac_tx_ctrl.pad_threshold = port_conf->pad_threashold;
    xlmac_tx_ctrl.tx_preamble_length = port_conf->tx_preamble_len;

    rc = rc ? rc : ag_drv_xport_xlmac_core_ctrl_set(portid, &xlmac_ctrl);
    rc = rc ? rc : ag_drv_xport_xlmac_core_tx_ctrl_set(portid, &xlmac_tx_ctrl);

    /* link speed related configuration should be done during link-up/xport_init */
    __xportDebug("DONE (rc = %d)\n", rc);
    return rc;
}

int xport_get_pause_configuration(uint32_t portid, xport_flow_ctrl_cfg_s *flow_ctrl)
{
    int rc;
    xport_xlmac_core_rx_ctrl xlmac_rx_ctrl;
    xport_xlmac_core_pause_ctrl pause_ctrl;

    rc = ag_drv_xport_xlmac_core_rx_ctrl_get(portid, &xlmac_rx_ctrl);
    rc = rc ? rc : ag_drv_xport_xlmac_core_tx_mac_sa_get(portid, &flow_ctrl->tx_ctrl_sa);
    rc = rc ? rc : ag_drv_xport_xlmac_core_rx_mac_sa_get(portid, &flow_ctrl->rx_ctrl_sa);
    rc = rc ? rc : ag_drv_xport_xlmac_core_pause_ctrl_get(portid, &pause_ctrl);

    flow_ctrl->pause_refresh_en = pause_ctrl.pause_refresh_en;
    flow_ctrl->pause_refresh_timer = pause_ctrl.pause_refresh_timer;
    flow_ctrl->pause_xoff_timer = pause_ctrl.pause_xoff_timer;
    flow_ctrl->rx_pass_ctrl = xlmac_rx_ctrl.rx_pass_ctrl;
    flow_ctrl->rx_pass_pause = xlmac_rx_ctrl.rx_pass_pause;
    flow_ctrl->rx_pause_en = pause_ctrl.rx_pause_en;
    flow_ctrl->tx_pause_en = pause_ctrl.tx_pause_en;

    __xportDebug("DONE (rc = %d)\n", rc);
    return rc;
}

int xport_set_pfc_configuration(uint32_t portid, xport_flow_ctrl_cfg_s *flow_ctrl)
{
    int rc;

    xport_xlmac_core_rx_ctrl xlmac_rx_ctrl;

    rc = ag_drv_xport_xlmac_core_rx_ctrl_get(portid, &xlmac_rx_ctrl);
    rc = rc ? rc : ag_drv_xport_xlmac_core_tx_mac_sa_set(portid, flow_ctrl->tx_ctrl_sa);
    rc = rc ? rc : ag_drv_xport_xlmac_core_rx_mac_sa_set(portid, flow_ctrl->rx_ctrl_sa);

    xlmac_rx_ctrl.rx_pass_ctrl = flow_ctrl->rx_pass_ctrl;

    rc = rc ? rc : ag_drv_xport_xlmac_core_rx_ctrl_set(portid, &xlmac_rx_ctrl);

    __xportDebug("DONE (rc = %d)\n", rc);
    return rc;
}

int xport_get_pfc_configuration(uint32_t portid, xport_flow_ctrl_cfg_s *flow_ctrl)
{
    int rc;
    xport_xlmac_core_rx_ctrl xlmac_rx_ctrl;

    rc = ag_drv_xport_xlmac_core_rx_ctrl_get(portid, &xlmac_rx_ctrl);
    flow_ctrl->rx_pass_ctrl = xlmac_rx_ctrl.rx_pass_ctrl;

    __xportDebug("DONE (rc = %d)\n", rc);
    return rc;
}

int xport_set_pause_configuration(uint32_t portid, xport_flow_ctrl_cfg_s *flow_ctrl)
{
    int rc;
    xport_xlmac_core_rx_ctrl xlmac_rx_ctrl;
    xport_xlmac_core_pause_ctrl pause_ctrl;

    rc = ag_drv_xport_xlmac_core_rx_ctrl_get(portid, &xlmac_rx_ctrl);
    rc = rc ? rc : ag_drv_xport_xlmac_core_tx_mac_sa_set(portid, flow_ctrl->tx_ctrl_sa);
    rc = rc ? rc : ag_drv_xport_xlmac_core_rx_mac_sa_set(portid, flow_ctrl->rx_ctrl_sa);
    rc = rc ? rc : ag_drv_xport_xlmac_core_pause_ctrl_get(portid, &pause_ctrl);

    pause_ctrl.pause_refresh_en = flow_ctrl->pause_refresh_en;
    pause_ctrl.pause_refresh_timer = flow_ctrl->pause_refresh_timer;
    pause_ctrl.pause_xoff_timer = flow_ctrl->pause_xoff_timer;
    xlmac_rx_ctrl.rx_pass_ctrl = flow_ctrl->rx_pass_ctrl;
    xlmac_rx_ctrl.rx_pass_pause = flow_ctrl->rx_pass_pause;
    pause_ctrl.rx_pause_en = flow_ctrl->rx_pause_en;
    pause_ctrl.tx_pause_en = flow_ctrl->tx_pause_en;

    rc = rc ? rc : ag_drv_xport_xlmac_core_rx_ctrl_set(portid, &xlmac_rx_ctrl);
    rc = rc ? rc : ag_drv_xport_xlmac_core_pause_ctrl_set(portid, &pause_ctrl);

    __xportDebug("DONE (rc = %d)\n", rc);
    return rc;
}

int xport_get_port_rxtx_enable(uint32_t portid, uint8_t *rx_en, uint8_t *tx_en)
{
    int rc;
    xport_xlmac_core_ctrl xlmac_ctrl;

    rc = ag_drv_xport_xlmac_core_ctrl_get(portid, &xlmac_ctrl);

    *rx_en = xlmac_ctrl.rx_en;
    *tx_en = xlmac_ctrl.tx_en;

    __xportDebug("DONE (rc = %d)\n", rc);
    return rc ;
}

int xport_set_port_rxtx_enable(uint32_t portid, uint8_t rx_en, uint8_t tx_en)
{
    int rc;
    xport_xlmac_core_ctrl xlmac_ctrl;

    rc = ag_drv_xport_xlmac_core_ctrl_get(portid, &xlmac_ctrl);

    xlmac_ctrl.rx_en = rx_en;
    xlmac_ctrl.tx_en = tx_en;

    return rc ? rc : ag_drv_xport_xlmac_core_ctrl_set(portid, &xlmac_ctrl);
}

int  xport_get_phyid(uint32_t port, uint16_t *phyid)
{
    __xportError("Not implemented\n");
    return 0;
}

int xport_reset_phy_cfg(uint32_t port, xport_port_phycfg_s *phycfg)
{
    __xportError("Not implemented\n");
    return 0;
}

/*This status analysis is for Broadcom's PHYs only as it described in the EGPHY register specification */
int xport_get_brcm_phy_status(uint16_t phyid,xport_port_status_s *port_status)
{
    __xportError("Not implemented\n");
    return 0;
}

int xport_get_port_status(uint32_t port, xport_port_status_s *port_status)
{
    int rc = XPORT_ERR_OK;
    xport_xlmac_core_ctrl ctrl;
    ag_drv_xport_xlmac_core_ctrl_get(port, &ctrl);
    port_status->mac_lpbk = ctrl.local_lpbk;
    port_status->mac_rx_en = ctrl.rx_en;
    port_status->mac_tx_en = ctrl.tx_en;
    port_status->rate = xport_speed_get(port);
    if (port == 0) /* XGMII 10G Serdes */
    {
        port_status->duplex = XPORT_FULL_DUPLEX;
    }
    else
    {
        xport_reg_crossbar_status crossbar_status;
        ag_drv_xport_reg_crossbar_status_get(&crossbar_status);
        port_status->duplex = crossbar_status.full_duplex ? XPORT_FULL_DUPLEX : XPORT_HALF_DUPLEX;
        port_status->port_up = crossbar_status.link_status;
    }
    {
        xport_xlmac_core_pause_ctrl pause_ctrl;
        ag_drv_xport_xlmac_core_pause_ctrl_get(port,&pause_ctrl);
        port_status->rx_pause_en = pause_ctrl.rx_pause_en;
        port_status->tx_pause_en = pause_ctrl.tx_pause_en;
    }

    return rc;
}

int xport_port_mtu_get(uint32_t portid, uint16_t *port_mtu)
{
    return ag_drv_xport_xlmac_core_rx_max_size_get(portid, port_mtu);
}

int xport_port_mtu_set(uint32_t portid, uint16_t port_mtu)
{
    uint8_t instance = portid % 4;

    /* set the XLMAC MTU */
    ag_drv_xport_xlmac_core_rx_max_size_set(portid, port_mtu);

    /* align the MIB max packet size for accurate accounting */
    switch (instance)
    {
    case 0:
        return ag_drv_xport_mib_reg_gport0_max_pkt_size_set(port_mtu);
    case 1:
        return ag_drv_xport_mib_reg_gport1_max_pkt_size_set(port_mtu);
    case 2:
        return ag_drv_xport_mib_reg_gport2_max_pkt_size_set(port_mtu);
    case 3:
        return ag_drv_xport_mib_reg_gport3_max_pkt_size_set(port_mtu);
    default:
        __xportError("(%d):Wrong portid %d",__LINE__, portid);
        return XPORT_ERR_PARAM;
    }
}

int xport_get_port_link_status(uint32_t port, uint8_t *link_up)
{
    __xportError("Not implemented\n");
    return XPORT_ERR_OK;
}

int xport_port_eee_set(uint32_t portid, uint8_t enable)
{
    XPORT_PORT_RATE xport_speed;
    uint32_t interface_id = XPORT_PORT_ID_2_INTF_TYPE(portid); /* What does interface_id mean here ? */
    uint16_t ref_count;
    uint16_t wake_timer;
    uint16_t delay_entry_timer = 34; // wait 34 us in EMPTY state before moving to LPI 
    uint8_t msbus_clk_sel, wan_led0_sel, timeout_rst_disable, p0_mode;
    int rc = 0;

    xport_speed = xport_speed_get(portid);

    /* set wait frim from LPI to active based on connection speed, numbers came from IEEE 802.3az */
    /* extra 1us buffer is added since the timer is not accurate */
    switch (xport_speed)
    {
        case XPORT_RATE_10G:
            /* 
             * 7.36us for 10Gb/s 10GBase-T
             * 4.48us for 10Gb/s 10GBase-T 
             * 12.38us for 10Gb/s XFI
             * 15.38us for 10Gb/s KR, no FEC
             * 17.38us for 10Gb/s KR with FEC
             */
            wake_timer = 19;
        break;
        case XPORT_RATE_2500MB:
            /* 
             * 29.44us for 2.5GBase-T
             */
            wake_timer = 31;
        break;
        case XPORT_RATE_1000MB:
            /* 
             * 16.5us for 1Gb/s
             */
            wake_timer = 18;
        break;
        case XPORT_RATE_100MB:
            /* 
             * 30us for 100 Mb/s
             */
            wake_timer = 31;
        break;
        case XPORT_RATE_10MB:
            __xportNotice("EEE for connection speed %s is not supported; "
                          "ignore setting eee to %d for xport mac %d\n",
                          xport_rate_to_str(xport_speed), enable, portid);
            return 0;
        break;
        default:
            /* connection speed not supported, return an error */
            rc = -1;
        break;
    }

    if (rc == 0)
    {
        /* obtain reference clock for timers */
        rc = ag_drv_xport_reg_xport_cntrl_1_get(&msbus_clk_sel, &wan_led0_sel, &timeout_rst_disable, &p0_mode);
        if (rc == 0)
        {
            if (msbus_clk_sel == 0)
            {
                ref_count = 500; // clock running at 500MHz, 500 count for 1 us
            }
            else
            {
                ref_count = 645; // clock running at 644.5MHz, 645 count for 1 us
            }
        }
    }

    if (rc == 0)
    {
        // BBH WAN register
        WANBBH->txBlk[interface_id].lan_config.eee = enable;

        // enable EEE on XPORT XLMAC
        ag_drv_xport_xlmac_core_eee_ctrl_set (portid, 0, enable);
        // set EEE timer in XPORT XLMAC
        ag_drv_xport_xlmac_core_eee_timers_set (portid, ref_count, wake_timer, delay_entry_timer);

        // Note : even this function is called 1 second after the PHY is enabled,
        // the 1 second link status timer will be set since this is how HW was validated
        ag_drv_xport_xlmac_core_eee_1_sec_link_status_timer_set (portid, 1000000);
    }
    return rc;
}

char *xport_rate_to_str(XPORT_PORT_RATE rate)
{
    switch (rate)
    {
    case XPORT_RATE_10MB: return "10M";
    case XPORT_RATE_100MB: return "100M";
    case XPORT_RATE_1000MB: return "1G";
    case XPORT_RATE_2500MB: return "2.5G";
    case XPORT_RATE_10G: return "10G";
    default: return "Unknown";
    }
    return "Unkown";
}

int xport_str_to_rate(char *str)
{
    if (!strcmp(str, "10M"))
        return XPORT_RATE_10MB;
    if (!strcmp(str, "100M"))
        return XPORT_RATE_100MB;
    if (!strcmp(str, "1G"))
        return XPORT_RATE_1000MB;
    if (!strcmp(str, "2.5G"))
        return XPORT_RATE_2500MB;
    if (!strcmp(str, "10G"))
        return XPORT_RATE_10G;

    return XPORT_RATE_UNKNOWN;
}

