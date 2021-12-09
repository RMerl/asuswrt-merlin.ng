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

/* Initialize the global structure with INVALID Interface type */
static xport_info_s g_xport_info = {.port_info[0 ... (XPORT_NUM_OF_PORTS-1)].intf_type = XPORT_INTF_INVALID};


mac_speed_t xport_rate_2_mac_speed(XPORT_PORT_RATE xport_rate)
{
    mac_speed_t mac_speed = MAC_SPEED_UNKNOWN;
    if (xport_rate == XPORT_RATE_10MB)
        mac_speed = MAC_SPEED_10;
    if (xport_rate== XPORT_RATE_100MB)
        mac_speed = MAC_SPEED_100;
    if (xport_rate== XPORT_RATE_1000MB)
        mac_speed = MAC_SPEED_1000;
    if (xport_rate== XPORT_RATE_2500MB)
        mac_speed = MAC_SPEED_2500;
    if (xport_rate== XPORT_RATE_10G)
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
        xport_rate = XPORT_RATE_10G;
    else if (mac_speed == MAC_SPEED_10000)
        xport_rate = XPORT_RATE_10G;
    return xport_rate;
}
static int port_xport_init(mac_dev_t *mac_dev)
{
    xport_stats_cfg_s stats_cfg = {}; 
    XPORT_INTF_TYPE intf_type = (uintptr_t)mac_dev->priv; 
    XPORT_PORT_ID port_id = XPORT_INTF_TYPE_2_PORT_ID(intf_type);
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

static int port_xport_enable(mac_dev_t *mac_dev)
{
    XPORT_INTF_TYPE intf_type = (uintptr_t)mac_dev->priv; 
    XPORT_PORT_ID port_id = XPORT_INTF_TYPE_2_PORT_ID(intf_type);
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
    XPORT_INTF_TYPE intf_type = (uintptr_t)mac_dev->priv; 
    XPORT_PORT_ID port_id = XPORT_INTF_TYPE_2_PORT_ID(intf_type);
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
    XPORT_INTF_TYPE intf_type = (uintptr_t)mac_dev->priv; 
    XPORT_PORT_ID port_id = XPORT_INTF_TYPE_2_PORT_ID(intf_type);
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
    xport_port_cfg_s port_cfg = {};
    XPORT_INTF_TYPE intf_type = (uintptr_t)mac_dev->priv; 
    XPORT_PORT_ID port_id = XPORT_INTF_TYPE_2_PORT_ID(intf_type);
    xport_xlmac_port_info_s *p_info;

    xport_assert(XPORT_PORT_VALID(port_id));

    p_info = &g_xport_info.port_info[port_id];

    if (mac_cfg->status == MAC_LINK_UP) /* Link up */
    {
        p_info->port_rate = mac_speed_2_xport_rate(mac_cfg->speed);
        p_info->port_duplex = XPORT_FULL_DUPLEX;
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
    XPORT_INTF_TYPE intf_type = (uintptr_t)mac_dev->priv; 
    XPORT_PORT_ID port_id = XPORT_INTF_TYPE_2_PORT_ID(intf_type);
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
    XPORT_INTF_TYPE intf_type = (uintptr_t)mac_dev->priv; 
    XPORT_PORT_ID port_id = XPORT_INTF_TYPE_2_PORT_ID(intf_type);
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

static int port_xport_stats_get(mac_dev_t *mac_dev, mac_stats_t *mac_stats)
{
    xport_rx_stats_s rx_stats;
    xport_tx_stats_s tx_stats;
    XPORT_INTF_TYPE intf_type = (uintptr_t)mac_dev->priv; 
    XPORT_PORT_ID port_id = XPORT_INTF_TYPE_2_PORT_ID(intf_type);
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
    XPORT_INTF_TYPE intf_type = (uintptr_t)mac_dev->priv; 
    XPORT_PORT_ID port_id = XPORT_INTF_TYPE_2_PORT_ID(intf_type);
    xport_assert(XPORT_PORT_VALID(port_id));

    if (xport_port_mtu_set(port_id, mtu))
    {
        __xportError("Failed to set mtu to %d for xport mac %d\n", mtu, port_id);
        return -1;
    }

    return 0;
}

static int port_xport_eee_set(mac_dev_t *mac_dev, int enable)
{
    XPORT_INTF_TYPE intf_type = (uintptr_t)mac_dev->priv; 
    XPORT_PORT_ID port_id = XPORT_INTF_TYPE_2_PORT_ID(intf_type);
    xport_assert(XPORT_PORT_VALID(port_id));

    if (xport_port_eee_set(port_id, enable))
    {
        __xportError("Failed to set eee to %d for xport mac %d\n", enable, port_id);
        return -1;
    }

    return 0;
}

static int port_xport_stats_clear(mac_dev_t *mac_dev)
{
    XPORT_INTF_TYPE intf_type = (uintptr_t)mac_dev->priv; 
    XPORT_PORT_ID port_id = XPORT_INTF_TYPE_2_PORT_ID(intf_type);
    xport_assert(XPORT_PORT_VALID(port_id));

    if (xport_stats_reset(port_id))
    {
        __xportError("Failed to clear stats for xport mac %d\n", port_id);
        return -1;
    }

    return 0;
}

static int port_xport_dev_add(mac_dev_t *mac_dev)
{
    XPORT_INTF_TYPE intf_type = (uintptr_t)mac_dev->priv; 
    XPORT_PORT_ID xport_port = XPORT_INTF_TYPE_2_PORT_ID(intf_type);
    xport_xlmac_port_info_s *p_info;

    xport_assert(XPORT_INTF_VALID(intf_type));
    xport_assert(XPORT_PORT_VALID(xport_port));
    /* Make sure current Interface type is invalid */
    p_info = &g_xport_info.port_info[xport_port];
    xport_assert(!XPORT_INTF_VALID(p_info->intf_type));

    p_info->port_rate = XPORT_RATE_UNKNOWN;
    p_info->port_duplex = XPORT_FULL_DUPLEX;
    p_info->intf_type = intf_type;
    p_info->xport_port_id = xport_port;

    __xportNotice("intf_type = %d xport_port=%d\n",intf_type,xport_port);
    return 0;
}

static int port_xport_dev_del(mac_dev_t *mac_dev)
{
    XPORT_INTF_TYPE intf_type = (uintptr_t)mac_dev->priv; 
    XPORT_PORT_ID port_id = XPORT_INTF_TYPE_2_PORT_ID(intf_type);
    xport_xlmac_port_info_s *p_info;

    xport_assert(XPORT_PORT_VALID(port_id));

    p_info = &g_xport_info.port_info[port_id];
    p_info->intf_type = XPORT_INTF_INVALID;
    p_info->port_rate = XPORT_RATE_UNKNOWN;

    return 0;
}

static int port_xport_drv_init(mac_drv_t *mac_drv)
{
    XPORT_PORT_ID port_id;
    xport_xlmac_port_info_s *p_info;

    for (port_id = XPORT_PORT_ID_AE; port_id < XPORT_NUM_OF_PORTS; port_id++)
    {
        p_info = &g_xport_info.port_info[port_id];
        if (XPORT_INTF_VALID(p_info->intf_type))
        {
            if (xport_init_driver(p_info))
            {
                __xportError("Failed to initialize the xport driver\n");
                return -1;
            }
        }
    }

    mac_drv->initialized = 1;

    return 0;
}

static int port_xport_loopback_set(mac_dev_t *mac_dev, mac_loopback_t loopback_set)
{
    xport_port_cfg_s port_cfg = {};
    XPORT_INTF_TYPE intf_type = (uintptr_t)mac_dev->priv; 
    XPORT_PORT_ID port_id = XPORT_INTF_TYPE_2_PORT_ID(intf_type);
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
    .enable = port_xport_enable,
    .disable = port_xport_disable,
    .cfg_get = port_xport_cfg_get,
    .cfg_set = port_xport_cfg_set,
    .pause_get = port_xport_pause_get,
    .pause_set = port_xport_pause_set,
    .stats_get = port_xport_stats_get,
    .stats_clear = port_xport_stats_clear,
    .mtu_set = port_xport_mtu_set,
    .eee_set = port_xport_eee_set,
    .dev_add = port_xport_dev_add,
    .dev_del = port_xport_dev_del,
    .drv_init = port_xport_drv_init,
    .loopback_set = port_xport_loopback_set,
};
