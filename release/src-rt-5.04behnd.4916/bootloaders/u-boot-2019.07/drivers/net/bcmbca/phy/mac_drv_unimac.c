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
#include "mac_drv_unimac.h"
#include "unimac_drv.h"         // for MAX_NUM_OF_EMACS
#include "dt_access.h"

static mac_stats_t cached_stats[MAX_NUM_OF_EMACS];

static int port_unimac_init(mac_dev_t *mac_dev)
{
    rdpa_emac emac = mac_dev->mac_id;
    unsigned long flags = (unsigned long)(mac_dev->priv);

    mac_hwapi_init_emac(emac);
    mac_hwapi_set_external_conf(emac, 0);
    mac_hwapi_set_unimac_cfg(emac, flags & UNIMAC_DRV_PRIV_FLAG_GMII_DIRECT);

    if (flags & UNIMAC_DRV_PRIV_FLAG_EXTSW_CONNECTED)
    {
        mac_hwapi_set_tx_min_pkt_size(emac, 64);
#if defined(CONFIG_BCM947622)
        // turn on rx fc by default to accept pause from external switch
        mac_dev->mac_drv->pause_set(mac_dev, 1, 0, NULL);
#else
        mac_hwapi_set_backpressure_ext(emac, 1);
#endif //!47622
    }
    if (flags & UNIMAC_DRV_PRIV_FLAG_SHRINK_IPG)
    {
        mac_hwapi_set_tx_igp_len(emac, 8);
    }

    return 0;
}

mac_speed_t unimac_rate_2_mac_speed(int unimac_rate)
{
    mac_speed_t mac_speed = MAC_SPEED_UNKNOWN;

    if (unimac_rate == UNIMAC_SPEED_10)
        mac_speed = MAC_SPEED_10;
    if (unimac_rate == UNIMAC_SPEED_100)
        mac_speed = MAC_SPEED_100;
    if (unimac_rate == UNIMAC_SPEED_1000)
        mac_speed = MAC_SPEED_1000;
    if (unimac_rate == UNIMAC_SPEED_2500)
        mac_speed = MAC_SPEED_2500;
    if (unimac_rate == UNIMAC_SPEED_5000)
        mac_speed = MAC_SPEED_5000;
    if (unimac_rate == UNIMAC_SPEED_10000)
        mac_speed = MAC_SPEED_10000;

    return mac_speed;
}

static int port_unimac_read_status(mac_dev_t *mac_dev, mac_status_t *mac_status)
{
    rdpa_emac emac = mac_dev->mac_id;
    S_HWAPI_MAC_STATUS macStatus;

    mac_hwapi_get_mac_status(emac, &macStatus);

    mac_status->link = macStatus.mac_link_stat;
    mac_status->speed = unimac_rate_2_mac_speed(macStatus.mac_speed);
	mac_status->duplex = macStatus.mac_duplex == 0 ? MAC_DUPLEX_FULL : MAC_DUPLEX_HALF;
	mac_status->pause_rx = macStatus.mac_rx_pause;
	mac_status->pause_tx = macStatus.mac_tx_pause;

    return 0;
}

static int port_unimac_enable(mac_dev_t *mac_dev)
{
    rdpa_emac emac = mac_dev->mac_id;

    mac_hwapi_set_rxtx_enable(emac, 1, 1);

    return 0;
}

static int port_unimac_disable(mac_dev_t *mac_dev)
{
    rdpa_emac emac = mac_dev->mac_id;

    mac_hwapi_set_rxtx_enable(emac, 0, 0);

    return 0;
}

static int port_unimac_cfg_get(mac_dev_t *mac_dev, mac_cfg_t *mac_cfg)
{
    rdpa_emac emac = mac_dev->mac_id;
    rdpa_emac_cfg_t emac_cfg = {};
    memset(mac_cfg, 0, sizeof(mac_cfg_t));

    mac_hwapi_get_configuration(emac, &emac_cfg);

    switch ((uint32_t)emac_cfg.rate) 
    {
    case UNIMAC_SPEED_10:       mac_cfg->speed = MAC_SPEED_10;      break;
    case UNIMAC_SPEED_100:      mac_cfg->speed = MAC_SPEED_100;     break;
    case UNIMAC_SPEED_1000:     mac_cfg->speed = MAC_SPEED_1000;    break;
    case UNIMAC_SPEED_2500:     mac_cfg->speed = MAC_SPEED_2500;    break;
    case UNIMAC_SPEED_5000:     mac_cfg->speed = MAC_SPEED_5000;    break;
    case UNIMAC_SPEED_10000:    mac_cfg->speed = MAC_SPEED_10000;   break;
    default:                    mac_cfg->speed = MAC_SPEED_UNKNOWN;
    }

    if (mac_cfg->speed != MAC_SPEED_UNKNOWN)
        mac_cfg->duplex = emac_cfg.full_duplex ? MAC_DUPLEX_FULL : MAC_DUPLEX_HALF;

    return 0;
}

static int port_unimac_cfg_set(mac_dev_t *mac_dev, mac_cfg_t *mac_cfg)
{
    rdpa_emac emac = mac_dev->mac_id;
    rdpa_emac_cfg_t emac_cfg = {};

    mac_hwapi_get_configuration(emac, &emac_cfg);

    switch (mac_cfg->speed)
    {
    case MAC_SPEED_10:      emac_cfg.rate = UNIMAC_SPEED_10;        break;
    case MAC_SPEED_100:     emac_cfg.rate = UNIMAC_SPEED_100;       break;
    case MAC_SPEED_1000:    emac_cfg.rate = UNIMAC_SPEED_1000;      break;
    case MAC_SPEED_2500:    emac_cfg.rate = UNIMAC_SPEED_2500;      break;
    case MAC_SPEED_5000:    emac_cfg.rate = UNIMAC_SPEED_5000;      break;
    case MAC_SPEED_10000:   emac_cfg.rate = UNIMAC_SPEED_10000;     break;
    default:                return -1;
    }

    emac_cfg.full_duplex = mac_cfg->duplex == MAC_DUPLEX_FULL ? 1 : 0;

    mac_hwapi_set_configuration(emac, &emac_cfg);

    return 0;
}

static int port_unimac_pause_get(mac_dev_t *mac_dev, int *rx_enable, int *tx_enable)
{
    S_MAC_HWAPI_FLOW_CTRL flowControl;

    mac_hwapi_get_flow_control(mac_dev->mac_id, &flowControl);
    *rx_enable = flowControl.rxFlowEnable ? 1 : 0;
    *tx_enable = flowControl.txFlowEnable ? 1 : 0;

    return 0;
}

static int port_unimac_pause_set(mac_dev_t *mac_dev, int rx_enable, int tx_enable, char *src_addr)
{
    S_MAC_HWAPI_FLOW_CTRL flowControl;
    bdmf_mac_t addr = {};

    mac_hwapi_get_flow_control(mac_dev->mac_id, &flowControl);

    flowControl.rxFlowEnable = rx_enable ? 1 : 0;
    flowControl.txFlowEnable = tx_enable ? 1 : 0;

    if (src_addr)
        memcpy(addr.b, src_addr, sizeof(bdmf_mac_t));

    mac_hwapi_set_flow_control(mac_dev->mac_id, &flowControl);
    mac_hwapi_modify_flow_control_pause_pkt_addr(mac_dev->mac_id, addr);

    return 0;
}

#ifndef BRCM_64B_UMAC_MIB

static int port_unimac_stats_get(mac_dev_t *mac_dev, mac_stats_t *mac_stats)
{
    rdpa_emac_stat_t stats;
    rdpa_emac emac = mac_dev->mac_id;

    /* TODO: lock */
    /* rdpa counters are read and clear, need to store data into local cache */
    mac_hwapi_get_rx_counters(emac, &stats.rx);
    mac_hwapi_get_tx_counters(emac, &stats.tx);

    cached_stats[emac].rx_byte += stats.rx.byte;
    cached_stats[emac].rx_packet += stats.rx.packet;
    cached_stats[emac].rx_frame_64 += stats.rx.frame_64;
    cached_stats[emac].rx_frame_65_127 += stats.rx.frame_65_127;
    cached_stats[emac].rx_frame_128_255 += stats.rx.frame_128_255;
    cached_stats[emac].rx_frame_256_511 += stats.rx.frame_256_511;
    cached_stats[emac].rx_frame_512_1023 += stats.rx.frame_512_1023;
    cached_stats[emac].rx_frame_1024_1518 += stats.rx.frame_1024_1518;
    cached_stats[emac].rx_frame_1519_mtu += stats.rx.frame_1519_mtu;
    cached_stats[emac].rx_multicast_packet += stats.rx.multicast_packet;
    cached_stats[emac].rx_broadcast_packet += stats.rx.broadcast_packet;
    cached_stats[emac].rx_unicast_packet += stats.rx.unicast_packet;
    cached_stats[emac].rx_alignment_error += stats.rx.alignment_error;
    cached_stats[emac].rx_frame_length_error += stats.rx.frame_length_error;
    cached_stats[emac].rx_code_error += stats.rx.code_error;
    cached_stats[emac].rx_carrier_sense_error += stats.rx.carrier_sense_error;
    cached_stats[emac].rx_fcs_error += stats.rx.fcs_error;
    cached_stats[emac].rx_undersize_packet += stats.rx.undersize_packet;
    cached_stats[emac].rx_oversize_packet += stats.rx.oversize_packet;
    cached_stats[emac].rx_fragments += stats.rx.fragments;
    cached_stats[emac].rx_jabber += stats.rx.jabber;
    cached_stats[emac].rx_overflow += stats.rx.overflow;
    cached_stats[emac].rx_control_frame += stats.rx.control_frame;
    cached_stats[emac].rx_pause_control_frame += stats.rx.pause_control_frame;
    cached_stats[emac].rx_unknown_opcode += stats.rx.unknown_opcode;
    cached_stats[emac].tx_byte += stats.tx.byte;
    cached_stats[emac].tx_packet += stats.tx.packet;
    cached_stats[emac].tx_frame_64 += stats.tx.frame_64;
    cached_stats[emac].tx_frame_65_127 += stats.tx.frame_65_127;
    cached_stats[emac].tx_frame_128_255 += stats.tx.frame_128_255;
    cached_stats[emac].tx_frame_256_511 += stats.tx.frame_256_511;
    cached_stats[emac].tx_frame_512_1023 += stats.tx.frame_512_1023;
    cached_stats[emac].tx_frame_1024_1518 += stats.tx.frame_1024_1518;
    cached_stats[emac].tx_frame_1519_mtu += stats.tx.frame_1519_mtu;
    cached_stats[emac].tx_fcs_error += stats.tx.fcs_error;
    cached_stats[emac].tx_multicast_packet += stats.tx.multicast_packet;
    cached_stats[emac].tx_broadcast_packet += stats.tx.broadcast_packet;
    cached_stats[emac].tx_unicast_packet += stats.tx.unicast_packet;
    cached_stats[emac].tx_total_collision += stats.tx.total_collision;
    cached_stats[emac].tx_jabber_frame += stats.tx.jabber_frame;
    cached_stats[emac].tx_oversize_frame += stats.tx.oversize_frame;
    cached_stats[emac].tx_undersize_frame += stats.tx.undersize_frame;
    cached_stats[emac].tx_fragments_frame += stats.tx.fragments_frame;
    cached_stats[emac].tx_error += stats.tx.error;
    cached_stats[emac].tx_underrun += stats.tx.underrun;
    cached_stats[emac].tx_excessive_collision += stats.tx.excessive_collision;
    cached_stats[emac].tx_late_collision += stats.tx.late_collision;
    cached_stats[emac].tx_single_collision += stats.tx.single_collision;
    cached_stats[emac].tx_multiple_collision += stats.tx.multiple_collision;
    cached_stats[emac].tx_pause_control_frame += stats.tx.pause_control_frame;
    cached_stats[emac].tx_deferral_packet += stats.tx.deferral_packet;
    cached_stats[emac].tx_excessive_deferral_packet += stats.tx.excessive_deferral_packet;
    cached_stats[emac].tx_control_frame += stats.tx.control_frame;

    memcpy(mac_stats, &cached_stats[emac], sizeof(mac_stats_t));

    /* TODO: unlock */

    return 0;
}

static int port_unimac_stats_clear(mac_dev_t *mac_dev)
{
    rdpa_emac emac = mac_dev->mac_id;
    rdpa_emac_stat_t stats;
    
    mac_hwapi_get_rx_counters(emac, &stats.rx);
    mac_hwapi_get_tx_counters(emac, &stats.tx);

    memset(&cached_stats[emac], 0, sizeof(cached_stats[0]));

    return 0;
}

#else /* BRCM_64B_UMAC_MIB */

static int port_unimac_stats_get(mac_dev_t *mac_dev, mac_stats_t *mac_stats)
{
    rdpa_emac_stat64_t stats;
    rdpa_emac emac = mac_dev->mac_id;

    /* TODO: lock */
    /* rdpa counters are read and clear, need to store data into local cache */
    mac_hwapi_get_rx_counters64(emac, &stats.rx);
    mac_hwapi_get_tx_counters64(emac, &stats.tx);

    cached_stats[emac].rx_byte += stats.rx.byte;
    cached_stats[emac].rx_packet += stats.rx.packet;
    cached_stats[emac].rx_frame_64 += stats.rx.frame_64;
    cached_stats[emac].rx_frame_65_127 += stats.rx.frame_65_127;
    cached_stats[emac].rx_frame_128_255 += stats.rx.frame_128_255;
    cached_stats[emac].rx_frame_256_511 += stats.rx.frame_256_511;
    cached_stats[emac].rx_frame_512_1023 += stats.rx.frame_512_1023;
    cached_stats[emac].rx_frame_1024_1518 += stats.rx.frame_1024_1518;
    cached_stats[emac].rx_frame_1519_mtu += stats.rx.frame_1519_mtu;
    cached_stats[emac].rx_multicast_packet += stats.rx.multicast_packet;
    cached_stats[emac].rx_broadcast_packet += stats.rx.broadcast_packet;
    cached_stats[emac].rx_unicast_packet += stats.rx.unicast_packet;
    cached_stats[emac].rx_alignment_error += stats.rx.alignment_error;
    cached_stats[emac].rx_frame_length_error += stats.rx.frame_length_error;
    cached_stats[emac].rx_code_error += stats.rx.code_error;
    cached_stats[emac].rx_carrier_sense_error += stats.rx.carrier_sense_error;
    cached_stats[emac].rx_fcs_error += stats.rx.fcs_error;
    cached_stats[emac].rx_undersize_packet += stats.rx.undersize_packet;
    cached_stats[emac].rx_oversize_packet += stats.rx.oversize_packet;
    cached_stats[emac].rx_fragments += stats.rx.fragments;
    cached_stats[emac].rx_jabber += stats.rx.jabber;
    cached_stats[emac].rx_overflow += stats.rx.overflow;
    cached_stats[emac].rx_control_frame += stats.rx.control_frame;
    cached_stats[emac].rx_pause_control_frame += stats.rx.pause_control_frame;
    cached_stats[emac].rx_unknown_opcode += stats.rx.unknown_opcode;
    cached_stats[emac].tx_byte += stats.tx.byte;
    cached_stats[emac].tx_packet += stats.tx.packet;
    cached_stats[emac].tx_frame_64 += stats.tx.frame_64;
    cached_stats[emac].tx_frame_65_127 += stats.tx.frame_65_127;
    cached_stats[emac].tx_frame_128_255 += stats.tx.frame_128_255;
    cached_stats[emac].tx_frame_256_511 += stats.tx.frame_256_511;
    cached_stats[emac].tx_frame_512_1023 += stats.tx.frame_512_1023;
    cached_stats[emac].tx_frame_1024_1518 += stats.tx.frame_1024_1518;
    cached_stats[emac].tx_frame_1519_mtu += stats.tx.frame_1519_mtu;
    cached_stats[emac].tx_fcs_error += stats.tx.fcs_error;
    cached_stats[emac].tx_multicast_packet += stats.tx.multicast_packet;
    cached_stats[emac].tx_broadcast_packet += stats.tx.broadcast_packet;
    cached_stats[emac].tx_unicast_packet += stats.tx.unicast_packet;
    cached_stats[emac].tx_total_collision += stats.tx.total_collision;
    cached_stats[emac].tx_jabber_frame += stats.tx.jabber_frame;
    cached_stats[emac].tx_oversize_frame += stats.tx.oversize_frame;
    cached_stats[emac].tx_undersize_frame += stats.tx.undersize_frame;
    cached_stats[emac].tx_fragments_frame += stats.tx.fragments_frame;
    cached_stats[emac].tx_error += stats.tx.error;
    cached_stats[emac].tx_underrun += stats.tx.underrun;
    cached_stats[emac].tx_excessive_collision += stats.tx.excessive_collision;
    cached_stats[emac].tx_late_collision += stats.tx.late_collision;
    cached_stats[emac].tx_single_collision += stats.tx.single_collision;
    cached_stats[emac].tx_multiple_collision += stats.tx.multiple_collision;
    cached_stats[emac].tx_pause_control_frame += stats.tx.pause_control_frame;
    cached_stats[emac].tx_deferral_packet += stats.tx.deferral_packet;
    cached_stats[emac].tx_excessive_deferral_packet += stats.tx.excessive_deferral_packet;
    cached_stats[emac].tx_control_frame += stats.tx.control_frame;

    memcpy(mac_stats, &cached_stats[emac], sizeof(mac_stats_t));

    /* TODO: unlock */

    return 0;
}

static int port_unimac_stats_clear(mac_dev_t *mac_dev)
{
    rdpa_emac emac = mac_dev->mac_id;
    rdpa_emac_stat64_t stats;
    
    mac_hwapi_get_rx_counters64(emac, &stats.rx);
    mac_hwapi_get_tx_counters64(emac, &stats.tx);

    memset(&cached_stats[emac], 0, sizeof(cached_stats[0]));

    return 0;
}

#endif /* BRCM_64B_UMAC_MIB */

static int port_unimac_mtu_set(mac_dev_t *mac_dev, int mtu)
{
    rdpa_emac emac = mac_dev->mac_id;

    mac_hwapi_set_rx_max_frame_len(emac, mtu);

    return 0;
}

static int port_unimac_eee_set(mac_dev_t *mac_dev, int enable)
{
    rdpa_emac emac = mac_dev->mac_id;

    mac_hwapi_set_eee(emac, enable);

    return 0;
}

static int port_unimac_loopback_set(mac_dev_t *mac_dev, mac_loopback_t loopback_set)
{
    rdpa_emac emac = mac_dev->mac_id;
    MAC_LPBK loopback;

    switch (loopback_set)
    {
        case MAC_LOOPBACK_NONE:
            loopback = MAC_LPBK_NONE;
            break;

        case MAC_LOOPBACK_LOCAL:
            loopback = MAC_LPBK_LOCAL;
            break;

        case MAC_LOOPBACK_REMOTE:
            loopback = MAC_LPBK_REMOTE;
            break;

        case MAC_LOOPBACK_BOTH:
            loopback = MAC_LPBK_BOTH;
            break;

        default:
            return -1;
    }

    mac_hwapi_set_loopback(emac, loopback);

    return 0;
}

static int port_unimac_dt_priv(const dt_handle_t handle, int mac_id, void **priv)
{
    unsigned long unimac_flags = 0;

    if (dt_property_read_bool(handle, "gmii-direct"))
        unimac_flags |= UNIMAC_DRV_PRIV_FLAG_GMII_DIRECT;

    *priv = (void *)(unsigned long)unimac_flags;

    return 0;
}

mac_drv_t mac_drv_unimac =
{
    .mac_type = MAC_TYPE_UNIMAC,
    .name = "UNIMAC",
    .init = port_unimac_init,
    .read_status = port_unimac_read_status,
    .enable = port_unimac_enable,
    .disable = port_unimac_disable,
    .cfg_get = port_unimac_cfg_get,
    .cfg_set = port_unimac_cfg_set,
    .pause_get = port_unimac_pause_get,
    .pause_set = port_unimac_pause_set,
    .stats_get = port_unimac_stats_get,
    .stats_clear = port_unimac_stats_clear,
    .mtu_set = port_unimac_mtu_set,
    .eee_set = port_unimac_eee_set,
    .loopback_set = port_unimac_loopback_set,
    .dt_priv = port_unimac_dt_priv,
};

