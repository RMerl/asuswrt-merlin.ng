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
 */

#include "mac_drv.h"
#include "xport_drv.h"
#include "xport_stats.h"

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
        mac_dev->stats_refresh_interval = mac_dev_get_stats_refresh_interval(mac_dev, 48, 40, mac_cfg->speed);
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

static mac_stats_t cached_stats[XPORT_NUM_OF_XLMACS*XPORT_NUM_OF_PORTS_PER_XLMAC];

static int port_xport_stats_get(mac_dev_t *mac_dev, mac_stats_t *mac_stats)
{
    xport_rx_stats_s rx_stats;
    xport_tx_stats_s tx_stats;
    mac_stats_t *cached;
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

    cached = &cached_stats[port_id];
    cached->rx_byte += rx_stats.GRXBYT;
    cached->rx_packet += rx_stats.GRXPOK;
    cached->rx_frame_64 += rx_stats.GRX64;
    cached->rx_frame_65_127 += rx_stats.GRX127;
    cached->rx_frame_128_255 += rx_stats.GRX255;
    cached->rx_frame_256_511 += rx_stats.GRX511;
    cached->rx_frame_512_1023 += rx_stats.GRX1023;
    cached->rx_frame_1024_1518 += rx_stats.GRX1518;
    cached->rx_frame_1519_mtu += rx_stats.GRX1522 + rx_stats.GRX2047 + rx_stats.GRX4095 +
        rx_stats.GRX9216 + rx_stats.GRX16383;
    cached->rx_multicast_packet += rx_stats.GRXMCA;
    cached->rx_broadcast_packet += rx_stats.GRXBCA;
    cached->rx_unicast_packet += rx_stats.GRXUCA;
    cached->rx_alignment_error += rx_stats.GRXALN;
    cached->rx_frame_length_error += rx_stats.GRXFLR;
    cached->rx_code_error += rx_stats.GRXFRERR;
    cached->rx_carrier_sense_error += rx_stats.GRXFCR;
    cached->rx_fcs_error += rx_stats.GRXFCS;
    cached->rx_undersize_packet += rx_stats.GRXUND;
    cached->rx_oversize_packet += rx_stats.GRXOVR;
    cached->rx_fragments += rx_stats.GRXFRG;
    cached->rx_jabber += rx_stats.GRXJBR;
    cached->rx_overflow += rx_stats.GRXTRFU;
    cached->rx_control_frame += rx_stats.GRXCF;
    cached->rx_pause_control_frame += rx_stats.GRXPF;
    cached->rx_unknown_opcode += rx_stats.GRXUO;
    cached->tx_byte += tx_stats.GTXBYT;
    cached->tx_packet += tx_stats.GTXPOK;
    cached->tx_frame_64 += tx_stats.GTX64;
    cached->tx_frame_65_127 += tx_stats.GTX127;
    cached->tx_frame_128_255 += tx_stats.GTX255;
    cached->tx_frame_256_511 += tx_stats.GTX511;
    cached->tx_frame_512_1023 += tx_stats.GTX1023;
    cached->tx_frame_1024_1518 += tx_stats.GTX1518;
    cached->tx_frame_1519_mtu += tx_stats.GTX1522 + tx_stats.GTX2047 + tx_stats.GTX4095 +
        tx_stats.GTX9216 + tx_stats.GTX16383;
    cached->tx_fcs_error += tx_stats.GTXFCS;
    cached->tx_multicast_packet += tx_stats.GTXMCA;
    cached->tx_broadcast_packet += tx_stats.GTXBCA;
    cached->tx_unicast_packet += tx_stats.GTXUCA;
    cached->tx_total_collision += tx_stats.GTXNCL;
    cached->tx_jabber_frame += tx_stats.GTXJBR;
    cached->tx_oversize_frame += tx_stats.GTXOVR;
    cached->tx_undersize_frame += tx_stats.GTXRPKT;
    cached->tx_fragments_frame += tx_stats.GTXFRG;
    cached->tx_error += tx_stats.GTXERR;
    cached->tx_underrun += tx_stats.GTXUFL;
    cached->tx_excessive_collision += tx_stats.GTXXCL;
    cached->tx_late_collision += tx_stats.GTXLCL;
    cached->tx_single_collision += tx_stats.GTXSCL;
    cached->tx_multiple_collision += tx_stats.GTXMCL;
    cached->tx_pause_control_frame += tx_stats.GTXPF;
    cached->tx_deferral_packet += tx_stats.GTXDFR;
    cached->tx_excessive_deferral_packet += tx_stats.GTXEDF;
    cached->tx_control_frame += tx_stats.GTXCF;
    cached->tx_fifo_errors += tx_stats.GTXUFL;

    memcpy(mac_stats, cached, sizeof(mac_stats_t));

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

    memset(&cached_stats[port_id], 0, sizeof(cached_stats[0]));

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

#if !defined(CONFIG_BCM963158)
#include "pmc_core_api.h"
#include "pmc_xport.h"
#endif

static int port_xport_drv_init(mac_drv_t *mac_drv)
{
    int rc = 0, i;

#if !defined(CONFIG_BCM963158)
    for (i = 0; i < XPORT_NUM_OF_XLMACS; i++)
    {
        if (!(enabled_ports & (((1 << XPORT_NUM_OF_PORTS_PER_XLMAC) - 1) << (i * XPORT_NUM_OF_PORTS_PER_XLMAC))))
            continue;

        rc |= pmc_xport_power_on(i);
    }
#endif

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
        __xportError("Failed to get configuration for xport mac %d", port_id);
        return -1;
    }
    
    switch (loopback_set) {
    case MAC_LOOPBACK_NONE:     port_cfg.local_loopback = 0;    port_cfg.remote_loopback = 0; break;
    case MAC_LOOPBACK_LOCAL:    port_cfg.local_loopback = 1;    port_cfg.remote_loopback = 0; break;
    case MAC_LOOPBACK_REMOTE:   port_cfg.local_loopback = 0;    port_cfg.remote_loopback = 1; break;
    case MAC_LOOPBACK_BOTH:     port_cfg.local_loopback = 1;    port_cfg.remote_loopback = 1; break;
    default:
        __xportError("Not support the loopback setting %d", loopback_set);
        return -1;
    }
    
    if (xport_set_port_configuration(port_id, &port_cfg))
    {
        __xportError("Failed to set configuration for xport mac %d", port_id);
        return -1;
    }

    return 0;
}

static int port_xport_loopback_get(mac_dev_t *mac_dev, mac_loopback_t *op)
{
    xport_port_cfg_s port_cfg = {};
    XPORT_PORT_ID port_id = mac_dev->mac_id;
    xport_assert(XPORT_PORT_VALID(port_id));

    if (xport_get_port_configuration(port_id, &port_cfg))
    {
        __xportError("Failed to get configuration for xport mac %d", port_id);
        return -1;
    }
    
    if (port_cfg.local_loopback)
        *op = (port_cfg.remote_loopback) ? MAC_LOOPBACK_BOTH : MAC_LOOPBACK_LOCAL;
    else
        *op = (port_cfg.remote_loopback) ? MAC_LOOPBACK_REMOTE : MAC_LOOPBACK_NONE;

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
    .loopback_get = port_xport_loopback_get,
};
