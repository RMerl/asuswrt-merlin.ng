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

/*
 *  Created on: Dec 2015
 *      Author: yuval.raviv@broadcom.com
 */

#ifndef __MAC_DRV_H__
#define __MAC_DRV_H__

#include "os_dep.h"

extern int eee_enabled;

typedef struct
{
    uint64_t rx_byte; /**< Receive Byte Counter */
    uint64_t rx_packet; /**< Receive Packet Counter */
    uint64_t rx_frame_64; /**< Receive 64 Byte Frame Counter */
    uint64_t rx_frame_65_127; /**< Receive 65 to 127 Byte Frame Counter */
    uint64_t rx_frame_128_255; /**< Receive 128 to 255 Byte Frame Counter */
    uint64_t rx_frame_256_511; /**< Receive 256 to 511 Byte Frame Counter */
    uint64_t rx_frame_512_1023; /**< Receive 512 to 1023 Byte Frame Counter */
    uint64_t rx_frame_1024_1518; /**< Receive 1024 to 1518 Byte Frame Counter */
    uint64_t rx_frame_1519_mtu; /**< Receive 1519 to MTU  Frame Counter */
    uint64_t rx_multicast_packet; /**< Receive Multicast Packet */
    uint64_t rx_broadcast_packet; /**< Receive Broadcast Packet */
    uint64_t rx_unicast_packet; /**< Receive Unicast Packet */
    uint64_t rx_alignment_error; /**< Receive Alignment error */
    uint64_t rx_frame_length_error; /**< Receive Frame Length Error Counter */
    uint64_t rx_code_error; /**< Receive Code Error Counter */
    uint64_t rx_carrier_sense_error; /**< Receive Carrier sense error */
    uint64_t rx_fcs_error; /**< Receive FCS Error Counter */
    uint64_t rx_undersize_packet; /**< Receive Undersize Packet */
    uint64_t rx_oversize_packet; /**< Receive Oversize Packet */
    uint64_t rx_fragments; /**< Receive Fragments */
    uint64_t rx_jabber; /**< Receive Jabber counter */
    uint64_t rx_overflow; /**< Receive Overflow counter */
    uint64_t rx_control_frame; /**< Receive Control Frame Counter */
    uint64_t rx_pause_control_frame; /**< Receive Pause Control Frame */
    uint64_t rx_unknown_opcode; /**< Receive Unknown opcode */
    uint64_t rx_fifo_errors; /**< Receive fifo errors */
    uint64_t rx_dropped;

    uint64_t tx_byte; /**< Transmit Byte Counter */
    uint64_t tx_packet; /**< Transmit Packet Counter */
    uint64_t tx_frame_64; /**< Transmit 64 Byte Frame Counter */
    uint64_t tx_frame_65_127; /**< Transmit 65 to 127 Byte Frame Counter */
    uint64_t tx_frame_128_255; /**< Transmit 128 to 255 Byte Frame Counter */
    uint64_t tx_frame_256_511; /**< Transmit 256 to 511 Byte Frame Counter */
    uint64_t tx_frame_512_1023; /**< Transmit 512 to 1023 Byte Frame Counter */
    uint64_t tx_frame_1024_1518; /**< Transmit 1024 to 1518 Byte Frame Counter */
    uint64_t tx_frame_1519_mtu; /**< Transmit 1519 to MTU Frame Counter */
    uint64_t tx_fcs_error; /**< Transmit FCS Error */
    uint64_t tx_multicast_packet; /**< Transmit Multicast Packet */
    uint64_t tx_broadcast_packet; /**< Transmit Broadcast Packet */
    uint64_t tx_unicast_packet; /**< Transmit Unicast Packet */
    uint64_t tx_total_collision; /**< Transmit Total Collision Counter */
    uint64_t tx_jabber_frame; /**< Transmit Jabber Frame */
    uint64_t tx_oversize_frame; /**< Transmit Oversize Frame counter */
    uint64_t tx_undersize_frame; /**< Transmit Undersize Frame */
    uint64_t tx_fragments_frame; /**< Transmit Fragments Frame counter */
    uint64_t tx_error; /**< Transmission errors*/
    uint64_t tx_underrun; /**< Transmission underrun */
    uint64_t tx_excessive_collision; /**< Transmit Excessive collision counter */
    uint64_t tx_late_collision; /**< Transmit Late collision counter */
    uint64_t tx_single_collision; /**< Transmit Single collision frame counter */
    uint64_t tx_multiple_collision; /**< Transmit Multiple collision frame counter */
    uint64_t tx_pause_control_frame; /**< Transmit PAUSE Control Frame */
    uint64_t tx_deferral_packet; /**< Transmit Deferral Packet */
    uint64_t tx_excessive_deferral_packet; /**< Transmit Excessive Deferral Packet */
    uint64_t tx_control_frame; /**< Transmit Control Frame */
    uint64_t tx_fifo_errors; /**< Transmit fifo errors */
    uint64_t tx_dropped;
} mac_stats_t;

typedef enum
{
    MAC_SPEED_UNKNOWN,
    MAC_SPEED_10,
    MAC_SPEED_100,
    MAC_SPEED_1000,
    MAC_SPEED_2500,
    MAC_SPEED_5000,
    MAC_SPEED_10000,
} mac_speed_t;

typedef enum
{
    MAC_DUPLEX_UNKNOWN,
    MAC_DUPLEX_HALF,
    MAC_DUPLEX_FULL,
} mac_duplex_t;

typedef enum
{
    MAC_TYPE_UNKNOWN,
    MAC_TYPE_UNIMAC,
    MAC_TYPE_LPORT,
    MAC_TYPE_GMAC,
    MAC_TYPE_xEPON,
    MAC_TYPE_EPON_AE,
    MAC_TYPE_SF2,
    MAC_TYPE_XPORT,
    MAC_TYPE_xGPON,
    MAC_TYPE_MAX,
} mac_type_t;

typedef enum
{
    MAC_LOOPBACK_NONE,
    MAC_LOOPBACK_LOCAL,
    MAC_LOOPBACK_REMOTE,
    MAC_LOOPBACK_BOTH,
} mac_loopback_t;

typedef struct
{
    mac_speed_t speed;
    mac_duplex_t duplex;
} mac_cfg_t;

typedef struct mac_dev_s
{
    struct mac_drv_s *mac_drv;
    int mac_id;
    void *priv;
} mac_dev_t;

typedef struct mac_drv_s
{
    mac_type_t mac_type;
    char *name;
    int initialized;
    int (*init)(mac_dev_t *self);
    int (*enable)(mac_dev_t *self);
    int (*disable)(mac_dev_t *self);
    int (*cfg_get)(mac_dev_t *self, mac_cfg_t *mac_cfg);
    int (*cfg_set)(mac_dev_t *self, mac_cfg_t *mac_cfg);
    int (*pause_set)(mac_dev_t *self, int rx_enable, int tx_enable, char *src_addr);
    int (*pause_get)(mac_dev_t *self, int *rx_enable, int *tx_enable);
    int (*pfc_set)(mac_dev_t *self, int rx_enable, int tx_enable, char *src_addr);
    int (*pfc_get)(mac_dev_t *self, int *rx_enable, int *tx_enable);
    int (*stats_get)(mac_dev_t *self, mac_stats_t *mac_stats);
    int (*stats_clear)(mac_dev_t *self);
    int (*mtu_set)(mac_dev_t *self, int mtu);
    int (*eee_set)(mac_dev_t *self, int enable);
    int (*dev_add)(mac_dev_t *self);
    int (*dev_del)(mac_dev_t *self);
    int (*drv_init)(struct mac_drv_s *mac_drv);
    int (*loopback_set)(mac_dev_t *self, mac_loopback_t loopback_set);
} mac_drv_t;

mac_dev_t *mac_dev_add(mac_type_t mac_type, int mac_id, void *priv);
int mac_dev_del(mac_dev_t *mac_dev);

int mac_drivers_set(void);
int mac_drivers_init(void);
int mac_driver_set(mac_drv_t *mac_drv);
int mac_driver_init(mac_type_t mac_type);

static inline int mac_dev_init(mac_dev_t *mac_dev)
{
    if (!mac_dev->mac_drv->init)
        return 0;

    return mac_dev->mac_drv->init(mac_dev);
}

static inline int mac_dev_enable(mac_dev_t *mac_dev)
{
    if (!mac_dev->mac_drv->enable)
        return 0;

    return mac_dev->mac_drv->enable(mac_dev);
}

static inline int mac_dev_disable(mac_dev_t *mac_dev)
{
    if (!mac_dev->mac_drv->disable)
        return 0;

    return mac_dev->mac_drv->disable(mac_dev);
}

static inline int mac_dev_cfg_get(mac_dev_t *mac_dev, mac_cfg_t *mac_cfg)
{
    if (!mac_dev->mac_drv->cfg_get)
        return 0;

    return mac_dev->mac_drv->cfg_get(mac_dev, mac_cfg);
}

static inline int mac_dev_cfg_set(mac_dev_t *mac_dev, mac_cfg_t *mac_cfg)
{
    if (!mac_dev->mac_drv->cfg_set)
        return 0;

    return mac_dev->mac_drv->cfg_set(mac_dev, mac_cfg);
}

static inline int mac_dev_pause_get(mac_dev_t *mac_dev, int *rx_enable, int *tx_enable)
{
    if (!mac_dev->mac_drv->pause_get)
        return 0;

    return mac_dev->mac_drv->pause_get(mac_dev, rx_enable, tx_enable);
}

static inline int mac_dev_pfc_set(mac_dev_t *mac_dev, int rx_enable, int tx_enable, char *src_addr)
{
    if (!mac_dev->mac_drv->pfc_set)
        return 0;

    return mac_dev->mac_drv->pfc_set(mac_dev, rx_enable, tx_enable, src_addr);
}

static inline int mac_dev_pfc_get(mac_dev_t *mac_dev, int *rx_enable, int *tx_enable)
{
    if (!mac_dev->mac_drv->pfc_get)
        return 0;

    return mac_dev->mac_drv->pfc_get(mac_dev, rx_enable, tx_enable);
}

static inline int mac_dev_pause_set(mac_dev_t *mac_dev, int rx_enable, int tx_enable, char *src_addr)
{
    if (!mac_dev->mac_drv->pause_set)
        return 0;

    return mac_dev->mac_drv->pause_set(mac_dev, rx_enable, tx_enable, src_addr);
}

static inline int mac_dev_stats_get(mac_dev_t *mac_dev, mac_stats_t *mac_stats)
{
    if (!mac_dev->mac_drv->stats_get)
        return 0;

    return mac_dev->mac_drv->stats_get(mac_dev, mac_stats);
}

static inline int mac_dev_stats_clear(mac_dev_t *mac_dev)
{
    if (!mac_dev->mac_drv->stats_clear)
        return 0;

    return mac_dev->mac_drv->stats_clear(mac_dev);
}

static inline int mac_dev_mtu_set(mac_dev_t *mac_dev, int mtu)
{
    if (!mac_dev->mac_drv->mtu_set)
        return 0;

    return mac_dev->mac_drv->mtu_set(mac_dev, mtu);
}

static inline int mac_dev_eee_set(mac_dev_t *mac_dev, int enable)
{
    if (!mac_dev->mac_drv->eee_set)
        return 0;

    if (!eee_enabled)
        enable = 0;

    return mac_dev->mac_drv->eee_set(mac_dev, enable);
}

static inline int mac_dev_loopback_set(mac_dev_t *mac_dev, mac_loopback_t loopback_set)
{
    if (!mac_dev->mac_drv->loopback_set)
        return 0;
    
    return mac_dev->mac_drv->loopback_set(mac_dev, loopback_set);
}

static inline int mac_drv_dev_add(mac_dev_t *mac_dev)
{
    if (mac_dev->mac_drv->initialized)
        return 0;

    if (!mac_dev->mac_drv->dev_add)
        return 0;

    return mac_dev->mac_drv->dev_add(mac_dev);
}

static inline int mac_drv_dev_del(mac_dev_t *mac_dev)
{
    if (mac_dev->mac_drv->initialized)
        return 0;

    if (!mac_dev->mac_drv->dev_del)
        return 0;

    return mac_dev->mac_drv->dev_del(mac_dev);
}

static inline int mac_drv_init(mac_drv_t *mac_drv)
{
    if (mac_drv->initialized)
        return 0;

    if (!mac_drv->drv_init)
        return 0;

    return mac_drv->drv_init(mac_drv);
}

#endif

