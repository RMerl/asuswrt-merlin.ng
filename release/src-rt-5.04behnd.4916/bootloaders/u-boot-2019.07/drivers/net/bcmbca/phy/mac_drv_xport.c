// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2017 Broadcom Corporation
   All Rights Reserved

*/

/*
 */

#include "mac_drv.h"
#include "xport_drv.h"
#include "xport_stats.h"
#include "wol.h"
#ifdef CONFIG_BCMBCA_PMC_XPORT
#include "pmc_xport.h"
#endif

static uint32_t enabled_ports;

/* Initialize the global structure with XPORT_NUM_OF_PORTS xport_port_id */
static xport_info_s g_xport_info = {.port_info[0 ... (XPORT_NUM_OF_PORTS-1)].xport_port_id = XPORT_NUM_OF_PORTS};


mac_speed_t xport_rate_2_mac_speed(XPORT_PORT_RATE xport_rate)
{
    mac_speed_t mac_speed = MAC_SPEED_UNKNOWN;

    if (xport_rate == XPORT_RATE_10MB)
        mac_speed = MAC_SPEED_10;
    if (xport_rate == XPORT_RATE_100MB)
        mac_speed = MAC_SPEED_100;
    if (xport_rate == XPORT_RATE_1000MB)
        mac_speed = MAC_SPEED_1000;
    if (xport_rate == XPORT_RATE_2500MB)
        mac_speed = MAC_SPEED_2500;
    if (xport_rate == XPORT_RATE_5G)
        mac_speed = MAC_SPEED_5000;
    if (xport_rate == XPORT_RATE_10G)
        mac_speed = MAC_SPEED_10000;
    return mac_speed;
}

XPORT_PORT_RATE mac_speed_2_xport_rate(mac_speed_t mac_speed)
{
    XPORT_PORT_RATE xport_rate = XPORT_RATE_UNKNOWN;

    if (mac_speed == MAC_SPEED_10)
        xport_rate = XPORT_RATE_10MB;
    else if (mac_speed == MAC_SPEED_100)
        xport_rate = XPORT_RATE_100MB;
    else if (mac_speed == MAC_SPEED_1000)
        xport_rate = XPORT_RATE_1000MB;
    else if (mac_speed == MAC_SPEED_2500)
        xport_rate = XPORT_RATE_2500MB;
    else if (mac_speed == MAC_SPEED_5000)
        xport_rate = XPORT_RATE_5G;
    else if (mac_speed == MAC_SPEED_10000)
        xport_rate = XPORT_RATE_10G;
    return xport_rate;
}

static int port_xport_init(mac_dev_t *mac_dev)
{
    xport_stats_cfg_s stats_cfg = {}; 
    XPORT_PORT_ID port_id = mac_dev->mac_id; 
    xport_assert(XPORT_PORT_VALID(port_id));

    stats_cfg.cor_en = 0; /* Disable Clear-On-Read */
    stats_cfg.saturate_en = 0; /* Disable saturation on max i.e. allow wrap around */

    if (xport_stats_cfg_set(port_id, &stats_cfg))
    {
        __xportError("Failed to initialize stats configuration for xport mac %d\n", port_id);
        return -1;
    }

    return 0;
}

static int port_xport_read_status(mac_dev_t *mac_dev, mac_status_t *mac_status)
{
    XPORT_PORT_ID port_id = mac_dev->mac_id; 
    xport_port_status_s port_status;
    xport_assert(XPORT_PORT_VALID(port_id));

    if (xport_get_port_status(port_id, &port_status))
    {
        __xportError("Failed to get status for xport mac %d\n", port_id);
        return -1;
    }

    mac_status->link = port_status.port_up;
    mac_status->speed = xport_rate_2_mac_speed(port_status.rate);
    mac_status->duplex = port_status.duplex == XPORT_FULL_DUPLEX ? MAC_DUPLEX_FULL : MAC_DUPLEX_HALF;
    mac_status->pause_rx = port_status.rx_pause_en;
    mac_status->pause_tx = port_status.tx_pause_en;

    return 0;
}

static int port_xport_enable(mac_dev_t *mac_dev)
{
    XPORT_PORT_ID port_id = mac_dev->mac_id; 
    xport_assert(XPORT_PORT_VALID(port_id));

    if (xport_set_port_rxtx_enable(port_id, 1, 1))
    {
        __xportError("Failed to enable rx/tx for xport mac %d\n", port_id);
        return -1;
    }

    return 0;
}

static int port_xport_disable(mac_dev_t *mac_dev)
{
    XPORT_PORT_ID port_id = mac_dev->mac_id; 
    xport_assert(XPORT_PORT_VALID(port_id));

    if (xport_set_port_rxtx_enable(port_id, 0, 0))
    {
        __xportError("Failed to disable rx/tx for xport mac %d\n", port_id);
        return -1;
    }

    return 0;
}

static int port_xport_cfg_get(mac_dev_t *mac_dev, mac_cfg_t *mac_cfg)
{
    xport_port_cfg_s port_cfg = {};
    XPORT_PORT_ID port_id = mac_dev->mac_id; 
    xport_assert(XPORT_PORT_VALID(port_id));

    memset(mac_cfg, 0, sizeof(mac_cfg_t));

    if (xport_get_port_configuration(port_id, &port_cfg))
    {
        __xportError("Failed to get configuration for xport mac %d\n", port_id);
        return -1;
    }

    mac_cfg->speed = xport_rate_2_mac_speed(port_cfg.speed);

    return 0;
}

static int port_xport_cfg_set(mac_dev_t *mac_dev, mac_cfg_t *mac_cfg)
{
    XPORT_PORT_ID port_id = mac_dev->mac_id;
    xport_xlmac_port_info_s *p_info;

    xport_assert(XPORT_PORT_VALID(port_id));

    p_info = &g_xport_info.port_info[port_id];

    if (mac_cfg->speed != MAC_SPEED_UNKNOWN) /* Link up */
    {
        p_info->port_rate = mac_speed_2_xport_rate(mac_cfg->speed);
        p_info->port_duplex = XPORT_FULL_DUPLEX;
        p_info->xgmii_mode = (mac_cfg->flag & MAC_FLAG_XGMII) > 0;
        xport_handle_link_up(p_info);
    }
    else
    {
        xport_handle_link_dn(p_info);
    }

    return 0;
}

static int port_xport_pause_get(mac_dev_t *mac_dev, int *rx_enable, int *tx_enable)
{
    xport_flow_ctrl_cfg_s flow_ctrl = {};
    XPORT_PORT_ID port_id = mac_dev->mac_id;
    xport_assert(XPORT_PORT_VALID(port_id));

    if (xport_get_pause_configuration(port_id, &flow_ctrl))
    {
        __xportError("Failed to get flow ctrl for xport mac %d\n", port_id);
        return -1;
    }

    *rx_enable = flow_ctrl.rx_pause_en ? 1 : 0;
    *tx_enable = flow_ctrl.tx_pause_en ? 1 : 0;

    return 0;
}

static int port_xport_pause_set(mac_dev_t *mac_dev, int rx_enable, int tx_enable, char *src_addr)
{
    xport_flow_ctrl_cfg_s flow_ctrl = {};
    XPORT_PORT_ID port_id = mac_dev->mac_id;
    xport_assert(XPORT_PORT_VALID(port_id));

    if (xport_get_pause_configuration(port_id, &flow_ctrl))
    {
        __xportError("Failed to get flow ctrl for xport mac %d\n", port_id);
        return -1;
    }

    flow_ctrl.rx_pause_en = rx_enable ? 1 : 0;
    flow_ctrl.tx_pause_en = tx_enable ? 1 : 0;

    if (xport_set_pause_configuration(port_id, &flow_ctrl))
    {
        __xportError("Failed to set flow ctrl for xport mac %d\n", port_id);
        return -1;
    }

    return 0;
}

static int port_xport_pfc_set(mac_dev_t *mac_dev, int rx_enable, int tx_enable, char *src_addr)
{
    xport_flow_ctrl_cfg_s flow_ctrl = {};
    XPORT_PORT_ID port_id = mac_dev->mac_id;
    xport_assert(XPORT_PORT_VALID(port_id));

    if (xport_get_pause_configuration(port_id, &flow_ctrl))
    {
        __xportError("Failed to get flow ctrl for xport mac %d\n", port_id);
        return -1;
    }

    flow_ctrl.rx_pass_ctrl = tx_enable;

    if (xport_set_pfc_configuration(port_id, &flow_ctrl))
    {
        __xportError("Failed to set flow ctrl for xport mac %d\n", port_id);
        return -1;
    }

    return 0;
}

static int port_xport_pfc_get(mac_dev_t *mac_dev, int *rx_enable, int *tx_enable)
{
    xport_flow_ctrl_cfg_s flow_ctrl = {};
    XPORT_PORT_ID port_id = mac_dev->mac_id;
    xport_assert(XPORT_PORT_VALID(port_id));

    if (xport_get_pfc_configuration(port_id, &flow_ctrl))
    {
        __xportError("Failed to get flow ctrl for xport mac %d\n", port_id);
        return -1;
    }

    *rx_enable = 0;
    *tx_enable = flow_ctrl.rx_pass_ctrl;

    return 0;
}

static int port_xport_stats_get(mac_dev_t *mac_dev, mac_stats_t *mac_stats)
{
    xport_rx_stats_s rx_stats;
    xport_tx_stats_s tx_stats;
    XPORT_PORT_ID port_id = mac_dev->mac_id;
    xport_assert(XPORT_PORT_VALID(port_id));

    if (xport_stats_get_rx(port_id, &rx_stats))
    {
        __xportError("Failed to get rx counters for xport mac %d\n", port_id);
        return -1;
    }

    if (xport_stats_get_tx(port_id, &tx_stats))
    {
        __xportError("Failed to get tx counters for xport mac %d\n", port_id);
        return -1;
    }

    mac_stats->rx_byte = rx_stats.GRXBYT;
    mac_stats->rx_packet = rx_stats.GRXPOK;
    mac_stats->rx_frame_64 = rx_stats.GRX64;
    mac_stats->rx_frame_65_127 = rx_stats.GRX127;
    mac_stats->rx_frame_128_255 = rx_stats.GRX255;
    mac_stats->rx_frame_256_511 = rx_stats.GRX511;
    mac_stats->rx_frame_512_1023 = rx_stats.GRX1023;
    mac_stats->rx_frame_1024_1518 = rx_stats.GRX1518;
    mac_stats->rx_frame_1519_mtu = rx_stats.GRX1522 + rx_stats.GRX2047 + rx_stats.GRX4095 +
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
    mac_stats->tx_frame_1519_mtu = tx_stats.GTX1522 + tx_stats.GTX2047 + tx_stats.GTX4095 +
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

static int port_xport_mtu_set(mac_dev_t *mac_dev, int mtu)
{
    XPORT_PORT_ID port_id = mac_dev->mac_id;
    xport_assert(XPORT_PORT_VALID(port_id));

    if (xport_mtu_set(port_id, mtu))
    {
        __xportError("Failed to set mtu to %d for xport mac %d\n", mtu, port_id);
        return -1;
    }

    return 0;
}

static int port_xport_eee_set(mac_dev_t *mac_dev, int enable)
{
    XPORT_PORT_ID port_id = mac_dev->mac_id;
    xport_assert(XPORT_PORT_VALID(port_id));

    if (xport_eee_set(port_id, enable))
    {
        __xportError("Failed to set eee to %d for xport mac %d\n", enable, port_id);
        return -1;
    }

    return 0;
}

static int port_xport_stats_clear(mac_dev_t *mac_dev)
{
    XPORT_PORT_ID port_id = mac_dev->mac_id;
    xport_assert(XPORT_PORT_VALID(port_id));

    if (xport_stats_reset(port_id))
    {
        __xportError("Failed to clear stats for xport mac %d\n", port_id);
        return -1;
    }

    return 0;
}

static int port_xport_wol_enable(mac_dev_t *mac_dev, wol_params_t *wol_params)
{
    XPORT_PORT_ID port_id = mac_dev->mac_id;
    xport_assert(XPORT_PORT_VALID(port_id));

    if (xport_wol_enable(port_id, wol_params))
    {
        __xportError("Failed to enable WOL for xport mac %d\n", port_id);
        return -1;
    }

    return 0;
}

static int port_xport_dev_add(mac_dev_t *mac_dev)
{
    XPORT_PORT_ID port_id = mac_dev->mac_id;
    xport_xlmac_port_info_s *p_info;

    xport_assert(XPORT_PORT_VALID(port_id));

    p_info = &g_xport_info.port_info[port_id];
    /* Make sure current port is invalid */
    xport_assert(!XPORT_PORT_VALID(p_info->xport_port_id));

    p_info->port_rate = XPORT_RATE_UNKNOWN;
    p_info->port_duplex = XPORT_FULL_DUPLEX;
    p_info->xport_port_id = port_id;

    enabled_ports |= (1 << (port_id));

    __xportInfo("dev add port_id=%d\n",port_id);

    return 0;
}

static int port_xport_dev_del(mac_dev_t *mac_dev)
{
    XPORT_PORT_ID port_id = mac_dev->mac_id;
    xport_xlmac_port_info_s *p_info;

    xport_assert(XPORT_PORT_VALID(port_id));

    p_info = &g_xport_info.port_info[port_id];
    p_info->xport_port_id = XPORT_NUM_OF_PORTS;
    p_info->port_rate = XPORT_RATE_UNKNOWN;

    enabled_ports &= ~(1 << (port_id));


    __xportInfo("dev del port_id=%d\n",port_id);

    return 0;
}

static int _port_xport_drv_init(XPORT_PORT_ID port_id)
{
    xport_xlmac_port_info_s *p_info = &g_xport_info.port_info[port_id];

    if (XPORT_PORT_VALID(p_info->xport_port_id))
    {
        if (xport_init_driver(p_info))
        {
            __xportError("Failed to initialize the xport driver for port %d\n", port_id);
            return -1;
        }
    }

    return 0;
}

static int port_xport_drv_init(mac_drv_t *mac_drv)
{
    int rc = 0, i;

    for (i = 0; i < XPORT_NUM_OF_XLMACS; i++)
    {
        if (!(enabled_ports & (((1 << XPORT_NUM_OF_PORTS_PER_XLMAC) - 1) << (i * XPORT_NUM_OF_PORTS_PER_XLMAC))))
            continue;

#ifdef CONFIG_BCMBCA_PMC_XPORT
        rc |= pmc_xport_power_on(i);
#endif
    }

    for (i = 0; i < XPORT_NUM_OF_PORTS; i++)
    {
        if (!(enabled_ports & (1 << i)))
            continue;

        rc |= _port_xport_drv_init(i);

    }

    mac_drv->initialized = 1;

    return rc;
}

static int port_xport_loopback_set(mac_dev_t *mac_dev, mac_loopback_t loopback_set)
{
    xport_port_cfg_s port_cfg = {};
    XPORT_PORT_ID port_id = mac_dev->mac_id;
    xport_assert(XPORT_PORT_VALID(port_id));

    if (xport_get_port_configuration(port_id, &port_cfg))
    {
        __xportError("Failed to get configuration for xport mac %d\n", port_id);
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
        __xportError("Not support the loopback setting %d\n", loopback_set);
        return -1;
    }

    if (xport_set_port_configuration(port_id, &port_cfg))
    {
        __xportError("Failed to set configuration for xport mac %d\n", port_id);
        return -1;
    }

    return 0;
}

mac_drv_t mac_drv_xport =
{
    .mac_type = MAC_TYPE_XPORT,
    .name = "XPORT",
    .init = port_xport_init,
    .read_status = port_xport_read_status,
    .enable = port_xport_enable,
    .disable = port_xport_disable,
    .cfg_get = port_xport_cfg_get,
    .cfg_set = port_xport_cfg_set,
    .pause_get = port_xport_pause_get,
    .pause_set = port_xport_pause_set,
    .pfc_set = port_xport_pfc_set,
    .pfc_get = port_xport_pfc_get,
    .stats_get = port_xport_stats_get,
    .stats_clear = port_xport_stats_clear,
    .mtu_set = port_xport_mtu_set,
    .eee_set = port_xport_eee_set,
    .wol_enable = port_xport_wol_enable,
    .dev_add = port_xport_dev_add,
    .dev_del = port_xport_dev_del,
    .drv_init = port_xport_drv_init,
    .loopback_set = port_xport_loopback_set,
};
