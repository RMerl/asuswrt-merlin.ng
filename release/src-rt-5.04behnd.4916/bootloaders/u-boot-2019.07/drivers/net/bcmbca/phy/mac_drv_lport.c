// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2015 Broadcom Corporation
   All Rights Reserved

    
*/

/*
 *  Created on: Jan 2016
 *      Author: yuval.raviv@broadcom.com
 */

#include "mac_drv.h"
#include "phy_drv.h"
#include "lport_drv.h"
#include "lport_stats.h"
#include "dt_access.h"

phy_mii_type_t dt_get_phy_mode_mii_type(const dt_handle_t handle);

static lport_init_s lport_init = {};

static int port_lport_init(mac_dev_t *mac_dev)
{
    uint32_t port_id = mac_dev->mac_id;
    lport_stats_cfg_s stats_cfg = {};

    if (lport_stats_cfg_set(port_id, &stats_cfg))
    {
        printk("Failed to initialize stats configuration for lport mac %d\n", port_id);
        return -1;
    }

    return 0;
}

static int port_lport_read_status(mac_dev_t *mac_dev, mac_status_t *mac_status)
{
    uint32_t port_id = mac_dev->mac_id;
    lport_port_cfg_s port_cfg = {};
    lport_flow_ctrl_cfg_s flow_ctrl = {};
    uint8_t rx_enable, tx_enable;

    if (lport_get_port_rxtx_enable(port_id, &rx_enable, &tx_enable))
    {
        printk("Failed to get status for lport mac %d\n", port_id);
        return -1;
    }

    if (lport_get_port_configuration(port_id, &port_cfg))
    {
        printk("Failed to get configuration for lport mac %d\n", port_id);
        return -1;
    }

    if (lport_get_pause_configuration(port_id, &flow_ctrl))
    {
        printk("Failed to get flow ctrl for lport mac %d\n", port_id);
        return -1;
    }

    mac_status->link = rx_enable && tx_enable;

    if (port_cfg.speed == LPORT_RATE_10MB)
        mac_status->speed = MAC_SPEED_10;
    else if (port_cfg.speed== LPORT_RATE_100MB)
        mac_status->speed = MAC_SPEED_100;
    else if (port_cfg.speed== LPORT_RATE_1000MB)
        mac_status->speed = MAC_SPEED_1000;
    else if (port_cfg.speed== LPORT_RATE_2500MB)
        mac_status->speed = MAC_SPEED_2500;
    else if (port_cfg.speed== LPORT_RATE_10G)
        mac_status->speed = MAC_SPEED_10000;
    else
        mac_status->speed = MAC_SPEED_UNKNOWN;

    if (mac_status->speed != MAC_SPEED_UNKNOWN)
        mac_status->duplex = MAC_DUPLEX_FULL;

    mac_status->pause_rx = flow_ctrl.rx_pause_en ? 1 : 0;
    mac_status->pause_tx = flow_ctrl.tx_pause_en ? 1 : 0;

    return 0;
}

static int port_lport_enable(mac_dev_t *mac_dev)
{
    uint32_t port_id = mac_dev->mac_id;

    if (lport_set_port_rxtx_enable(port_id, 1, 1))
    {
        printk("Failed to enable rx/tx for lport mac %d\n", port_id);
        return -1;
    }

    return 0;
}

static int port_lport_disable(mac_dev_t *mac_dev)
{
    uint32_t port_id = mac_dev->mac_id;

    if (lport_set_port_rxtx_enable(port_id, 0, 0))
    {
        printk("Failed to disable rx/tx for lport mac %d\n", port_id);
        return -1;
    }

    return 0;
}

static int port_lport_cfg_get(mac_dev_t *mac_dev, mac_cfg_t *mac_cfg)
{
    uint32_t port_id = mac_dev->mac_id;
    lport_port_cfg_s port_cfg = {};
    lport_flow_ctrl_cfg_s flow_ctrl = {};
    memset(mac_cfg, 0, sizeof(mac_cfg_t));

    if (lport_get_port_configuration(port_id, &port_cfg))
    {
        printk("Failed to get configuration for lport mac %d\n", port_id);
        return -1;
    }

    if (lport_get_pause_configuration(port_id, &flow_ctrl))
    {
        printk("Failed to get flow ctrl for lport mac %d\n", port_id);
        return -1;
    }

    if (port_cfg.speed == LPORT_RATE_10MB)
        mac_cfg->speed = MAC_SPEED_10;
    else if (port_cfg.speed== LPORT_RATE_100MB)
        mac_cfg->speed = MAC_SPEED_100;
    else if (port_cfg.speed== LPORT_RATE_1000MB)
        mac_cfg->speed = MAC_SPEED_1000;
    else if (port_cfg.speed== LPORT_RATE_2500MB)
        mac_cfg->speed = MAC_SPEED_2500;
    else if (port_cfg.speed== LPORT_RATE_10G)
        mac_cfg->speed = MAC_SPEED_10000;
    else
        mac_cfg->speed = MAC_SPEED_UNKNOWN;

    if (mac_cfg->speed != MAC_SPEED_UNKNOWN)
        mac_cfg->duplex = MAC_DUPLEX_FULL;

    return 0;
}

static int port_lport_cfg_set(mac_dev_t *mac_dev, mac_cfg_t *mac_cfg)
{
    uint32_t port_id = mac_dev->mac_id;
    lport_port_cfg_s port_cfg = {};

    if (lport_get_port_configuration(port_id, &port_cfg))
    {
        printk("Failed to get configuration for lport mac %d\n", port_id);
        return -1;
    }

    if (mac_cfg->speed == MAC_SPEED_10)
        port_cfg.speed = LPORT_RATE_10MB;
    else if (mac_cfg->speed == MAC_SPEED_100)
        port_cfg.speed = LPORT_RATE_100MB;
    else if (mac_cfg->speed == MAC_SPEED_1000)
        port_cfg.speed = LPORT_RATE_1000MB;
    else if (mac_cfg->speed == MAC_SPEED_2500)
        port_cfg.speed = LPORT_RATE_2500MB;
    else if (mac_cfg->speed == MAC_SPEED_10000)
        port_cfg.speed = LPORT_RATE_10G;
    else
        port_cfg.speed = LPORT_RATE_UNKNOWN;

    port_cfg.throt_num = 0x00;
    port_cfg.throt_denom = 0x00;

    if (mac_cfg->speed == MAC_SPEED_5000)
    {
        port_cfg.throt_num = 0x3f;
        port_cfg.throt_denom = 0x3f;
        port_cfg.speed = LPORT_RATE_10G;
    }

    if (lport_set_port_configuration(port_id, &port_cfg))
    {
        printk("Failed to set configuration for lport mac %d\n", port_id);
        return -1;
    }

    return 0;
}

static int port_lport_pause_get(mac_dev_t *mac_dev, int *rx_enable, int *tx_enable)
{
    uint32_t port_id = mac_dev->mac_id;
    lport_flow_ctrl_cfg_s flow_ctrl = {};

    if (lport_get_pause_configuration(port_id, &flow_ctrl))
    {
        printk("Failed to get flow ctrl for lport mac %d\n", port_id);
        return -1;
    }

    *rx_enable = flow_ctrl.rx_pause_en ? 1 : 0;
    *tx_enable = flow_ctrl.tx_pause_en ? 1 : 0;

    return 0;
}

static int port_lport_pause_set(mac_dev_t *mac_dev, int rx_enable, int tx_enable, char *src_addr)
{
    uint32_t port_id = mac_dev->mac_id;
    lport_flow_ctrl_cfg_s flow_ctrl = {};
    char mac_addr[8] = {};
    int i;

    if (lport_get_pause_configuration(port_id, &flow_ctrl))
    {
        printk("Failed to get flow ctrl for lport mac %d\n", port_id);
        return -1;
    }

    flow_ctrl.rx_pause_en = rx_enable ? 1 : 0;
    flow_ctrl.tx_pause_en = tx_enable ? 1 : 0;

    if (src_addr)
    {
        for (i = 0; i < 6 ;i++)
            mac_addr[5 - i] = src_addr[i];

        flow_ctrl.tx_ctrl_sa = *(uint64_t *)mac_addr;
    }

    if (lport_set_pause_configuration(port_id, &flow_ctrl))
    {
        printk("Failed to set flow ctrl for lport mac %d\n", port_id);
        return -1;
    }

    return 0;
}

static int port_lport_stats_get(mac_dev_t *mac_dev, mac_stats_t *mac_stats)
{
    uint32_t port_id = mac_dev->mac_id;
    lport_rx_stats_s rx_stats;
    lport_tx_stats_s tx_stats;

    if (lport_stats_get_rx(port_id, &rx_stats))
    {
        printk("Failed to get rx counters for lport mac %d\n", port_id);
        return -1;
    }

    if (lport_stats_get_tx(port_id, &tx_stats))
    {
        printk("Failed to get tx counters for lport mac %d\n", port_id);
        return -1;
    }

    mac_stats->rx_byte = rx_stats.GRXBYT + rx_stats.GRXRBYT;
    mac_stats->rx_packet = rx_stats.GRXPOK;
    mac_stats->rx_frame_64 = rx_stats.GRX64;
    mac_stats->rx_frame_65_127 = rx_stats.GRX127;
    mac_stats->rx_frame_128_255 = rx_stats.GRX255;
    mac_stats->rx_frame_256_511 = rx_stats.GRX511;
    mac_stats->rx_frame_512_1023 = rx_stats.GRX1023;
    mac_stats->rx_frame_1024_1518 = rx_stats.GRX1518;
    mac_stats->rx_frame_1519_mtu = rx_stats.GRX2047 + rx_stats.GRX4095 +
        rx_stats.GRX9216 + rx_stats.GRX16383;
    mac_stats->rx_multicast_packet = rx_stats.GRXMCA;
    mac_stats->rx_broadcast_packet = rx_stats.GRXBCA;
    mac_stats->rx_unicast_packet = rx_stats.GRXUCA;
    mac_stats->rx_alignment_error = rx_stats.GRXALN;
    mac_stats->rx_frame_length_error = rx_stats.GRXFLR;
    mac_stats->rx_code_error = rx_stats.GRXFRERR;
    mac_stats->rx_carrier_sense_error = rx_stats.GRXFCR;
    mac_stats->rx_fcs_error = rx_stats.GRXFCS;
    mac_stats->rx_undersize_packet = rx_stats.GRXUND;
    mac_stats->rx_oversize_packet = rx_stats.GRXOVR;
    mac_stats->rx_fragments = rx_stats.GRXFRG;
    mac_stats->rx_jabber = rx_stats.GRXJBR;
    mac_stats->rx_overflow = rx_stats.GRXTRFU;
    mac_stats->rx_control_frame = rx_stats.GRXCF;
    mac_stats->rx_pause_control_frame = rx_stats.GRXPF;
    mac_stats->rx_unknown_opcode = rx_stats.GRXUO;
    mac_stats->tx_byte = tx_stats.GTXBYT;
    mac_stats->tx_packet = tx_stats.GTXPOK;
    mac_stats->tx_frame_64 = tx_stats.GTX64;
    mac_stats->tx_frame_65_127 = tx_stats.GTX127;
    mac_stats->tx_frame_128_255 = tx_stats.GTX255;
    mac_stats->tx_frame_256_511 = tx_stats.GTX511;
    mac_stats->tx_frame_512_1023 = tx_stats.GTX1023;
    mac_stats->tx_frame_1024_1518 = tx_stats.GTX1518;
    mac_stats->tx_frame_1519_mtu = tx_stats.GTX2047 + tx_stats.GTX4095 +
        tx_stats.GTX9216 + tx_stats.GTX16383;
    mac_stats->tx_fcs_error = tx_stats.GTXFCS;
    mac_stats->tx_multicast_packet = tx_stats.GTXMCA;
    mac_stats->tx_broadcast_packet = tx_stats.GTXBCA;
    mac_stats->tx_unicast_packet = tx_stats.GTXUCA;
    mac_stats->tx_total_collision = tx_stats.GTXNCL;
    mac_stats->tx_jabber_frame = tx_stats.GTXJBR;
    mac_stats->tx_oversize_frame = tx_stats.GTXOVR;
    mac_stats->tx_undersize_frame = tx_stats.GTXRPKT;
    mac_stats->tx_fragments_frame = tx_stats.GTXFRG;
    mac_stats->tx_error = tx_stats.GTXERR;
    mac_stats->tx_underrun = tx_stats.GTXUFL;
    mac_stats->tx_excessive_collision = tx_stats.GTXXCL;
    mac_stats->tx_late_collision = tx_stats.GTXLCL;
    mac_stats->tx_single_collision = tx_stats.GTXSCL;
    mac_stats->tx_multiple_collision = tx_stats.GTXMCL;
    mac_stats->tx_pause_control_frame = tx_stats.GTXPF;
    mac_stats->tx_deferral_packet = tx_stats.GTXDFR;
    mac_stats->tx_excessive_deferral_packet = tx_stats.GTXEDF;
    mac_stats->tx_control_frame = tx_stats.GTXCF;
    mac_stats->tx_fifo_errors = tx_stats.GTXUFL;

    return 0;
}

static int port_lport_mtu_set(mac_dev_t *mac_dev, int mtu)
{
    uint32_t port_id = mac_dev->mac_id;

    if (lport_port_mtu_set(port_id, mtu))
    {
        printk("Failed to set mtu to %d for lport mac %d\n", mtu, port_id);
        return -1;
    }

    return 0;
}

static int port_lport_eee_set(mac_dev_t *mac_dev, int enable)
{
    uint32_t port_id = mac_dev->mac_id;

    if (lport_port_eee_set(port_id, enable))
    {
        printk("Failed to set eee to %d for lport mac %d\n", enable, port_id);
        return -1;
    }

    return 0;
}

static int port_lport_stats_clear(mac_dev_t *mac_dev)
{
    uint32_t port_id = mac_dev->mac_id;

    if (lport_stats_reset(port_id))
    {
        printk("Failed to clear stats for lport mac %d\n", port_id);
        return -1;
    }

    return 0;
}

static int port_lport_dev_add(mac_dev_t *mac_dev)
{
    uint32_t port_id = mac_dev->mac_id;

    lport_init.prt_mux_sel[port_id] = (LPORT_PORT_MUX_SELECT)mac_dev->priv;

    return 0;
}

static int port_lport_dev_del(mac_dev_t *mac_dev)
{
    uint32_t port_id = mac_dev->mac_id;

    lport_init.prt_mux_sel[port_id] = PORT_UNAVAIL;

    return 0;
}

static int port_lport_drv_init(mac_drv_t *mac_drv)
{
    if (lport_init_driver(&lport_init))
    {
        printk("Failed to initialize the lport driver\n");
        return -1;
    }

    mac_drv->initialized = 1;

    return 0;
}

static int port_lport_loopback_set(mac_dev_t *mac_dev, mac_loopback_t loopback_set)
{
    uint32_t port_id = mac_dev->mac_id;
    lport_port_cfg_s port_cfg = {};

    if (lport_get_port_configuration(port_id, &port_cfg))
    {
        printk("Failed to get configuration for lport mac %d\n", port_id);
        return -1;
    }
    
    if (loopback_set == MAC_LOOPBACK_NONE)
    {
        port_cfg.local_loopback = 0;
    }
    else if (loopback_set == MAC_LOOPBACK_LOCAL)
    {
        port_cfg.local_loopback = 1;
    }
    else
    {
        printk("Not support the loopback setting %d\n", loopback_set);
        return -1;
    }
    
    if (lport_set_port_configuration(port_id, &port_cfg))
    {
        printk("Failed to set configuration for lport mac %d\n", port_id);
        return -1;
    }

    return 0;
}

static int port_lport_dt_priv(const dt_handle_t handle, int mac_id, void **priv)
{
    uint32_t mux_sel = PORT_UNAVAIL;
    phy_mii_type_t mii_type;
    const dt_handle_t phy_handle = dt_parse_phandle(handle, "phy-handle", 0);

    mii_type = dt_get_phy_mode_mii_type(handle);
    switch (mii_type)
    {
        case PHY_MII_TYPE_GMII:
            mux_sel = PORT_GPHY;
            break;
        case PHY_MII_TYPE_RGMII:
            mux_sel = PORT_RGMII;
            lport_init.rgmii_cfg[mac_id - LPORT_FIRST_RGMII_PORT].is_1p8v = dt_property_read_bool(handle, "rgmii-1p8v");
            lport_init.rgmii_cfg[mac_id - LPORT_FIRST_RGMII_PORT].delay_rx = dt_property_read_bool(handle, "rx-delay");
            lport_init.rgmii_cfg[mac_id - LPORT_FIRST_RGMII_PORT].delay_tx = dt_property_read_bool(handle, "tx-delay");
            lport_init.rgmii_cfg[mac_id - LPORT_FIRST_RGMII_PORT].phy_attached = dt_is_valid(phy_handle) ? 1 : 0;
            lport_init.has_rgmii_cfg = 1;
            break;
        case PHY_MII_TYPE_HSGMII:
            mux_sel = PORT_HSGMII;
            break;
        case PHY_MII_TYPE_XFI:
            mux_sel = PORT_XFI;
            break;
        case PHY_MII_TYPE_SGMII:
            mux_sel = PORT_SGMII;
            break;
        default:
            break;
    }

    *priv = (void *)(unsigned long)mux_sel;

    return 0;
}

mac_drv_t mac_drv_lport =
{
    .mac_type = MAC_TYPE_LPORT,
    .name = "LPORT",
    .init = port_lport_init,
    .read_status = port_lport_read_status,
    .enable = port_lport_enable,
    .disable = port_lport_disable,
    .cfg_get = port_lport_cfg_get,
    .cfg_set = port_lport_cfg_set,
    .pause_get = port_lport_pause_get,
    .pause_set = port_lport_pause_set,
    .stats_get = port_lport_stats_get,
    .stats_clear = port_lport_stats_clear,
    .mtu_set = port_lport_mtu_set,
    .eee_set = port_lport_eee_set,
    .dev_add = port_lport_dev_add,
    .dev_del = port_lport_dev_del,
    .drv_init = port_lport_drv_init,
    .loopback_set = port_lport_loopback_set,
    .dt_priv = port_lport_dt_priv,
};

