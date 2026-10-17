// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2024 Broadcom Corporation
   All Rights Reserved
*/

/*
 *	Created on: Dec 2015
 *		Author: yuval.raviv@broadcom.com
 */

#ifndef __MAC_DRV_H__
#define __MAC_DRV_H__

#include "os_dep.h"
#include "dt_access.h"
#include "brcm_rgmii.h"
#include "wol.h"

extern int eee_enabled;

#ifdef pr_cont
#undef pr_cont
#endif
#define pr_cont printk

typedef struct {
	u64 rx_byte; /**< Receive Byte Counter */
	u64 rx_packet; /**< Receive Packet Counter */
	u64 rx_frame_64; /**< Receive 64 Byte Frame Counter */
	u64 rx_frame_65_127; /**< Receive 65 to 127 Byte Frame Counter */
	u64 rx_frame_128_255; /**< Receive 128 to 255 Byte Frame Counter */
	u64 rx_frame_256_511; /**< Receive 256 to 511 Byte Frame Counter */
	u64 rx_frame_512_1023; /**< Receive 512 to 1023 Byte Frame Counter */
	u64 rx_frame_1024_1518; /**< Receive 1024 to 1518 Byte Frame Counter */
	u64 rx_frame_1519_mtu; /**< Receive 1519 to MTU Frame Counter */
	u64 rx_multicast_packet; /**< Receive Multicast Packet */
	u64 rx_broadcast_packet; /**< Receive Broadcast Packet */
	u64 rx_unicast_packet; /**< Receive Unicast Packet */
	u64 rx_alignment_error; /**< Receive Alignment error */
	u64 rx_frame_length_error; /**< Receive Frame Length Error Counter */
	u64 rx_code_error; /**< Receive Code Error Counter */
	u64 rx_carrier_sense_error; /**< Receive Carrier sense error */
	u64 rx_fcs_error; /**< Receive FCS Error Counter */
	u64 rx_undersize_packet; /**< Receive Undersize Packet */
	u64 rx_oversize_packet; /**< Receive Oversize Packet */
	u64 rx_fragments; /**< Receive Fragments */
	u64 rx_jabber; /**< Receive Jabber counter */
	u64 rx_overflow; /**< Receive Overflow counter */
	u64 rx_control_frame; /**< Receive Control Frame Counter */
	u64 rx_pause_control_frame; /**< Receive Pause Control Frame */
	u64 rx_unknown_opcode; /**< Receive Unknown opcode */
	u64 rx_fifo_errors; /**< Receive fifo errors */
	u64 rx_dropped;

	u64 tx_byte; /**< Transmit Byte Counter */
	u64 tx_packet; /**< Transmit Packet Counter */
	u64 tx_frame_64; /**< Transmit 64 Byte Frame Counter */
	u64 tx_frame_65_127; /**< Transmit 65 to 127 Byte Frame Counter */
	u64 tx_frame_128_255; /**< Transmit 128 to 255 Byte Frame Counter */
	u64 tx_frame_256_511; /**< Transmit 256 to 511 Byte Frame Counter */
	u64 tx_frame_512_1023; /**< Transmit 512 to 1023 Byte Frame Counter */
	u64 tx_frame_1024_1518; /**< Transmit 1024 to 1518 Byte Frame Counter */
	u64 tx_frame_1519_mtu; /**< Transmit 1519 to MTU Frame Counter */
	u64 tx_fcs_error; /**< Transmit FCS Error */
	u64 tx_multicast_packet; /**< Transmit Multicast Packet */
	u64 tx_broadcast_packet; /**< Transmit Broadcast Packet */
	u64 tx_unicast_packet; /**< Transmit Unicast Packet */
	u64 tx_total_collision; /**< Transmit Total Collision Counter */
	u64 tx_jabber_frame; /**< Transmit Jabber Frame */
	u64 tx_oversize_frame; /**< Transmit Oversize Frame counter */
	u64 tx_undersize_frame; /**< Transmit Undersize Frame */
	u64 tx_fragments_frame; /**< Transmit Fragments Frame counter */
	u64 tx_error; /**< Transmission errors*/
	u64 tx_underrun; /**< Transmission underrun */
	u64 tx_excessive_collision; /**< Transmit Excessive collision counter */
	u64 tx_late_collision; /**< Transmit Late collision counter */
	u64 tx_single_collision; /**< Transmit Single collision frame counter */
	u64 tx_multiple_collision; /**< Transmit Multiple collision frame counter */
	u64 tx_pause_control_frame; /**< Transmit PAUSE Control Frame */
	u64 tx_deferral_packet; /**< Transmit Deferral Packet */
	u64 tx_excessive_deferral_packet; /**< Transmit Excessive Deferral Packet */
	u64 tx_control_frame; /**< Transmit Control Frame */
	u64 tx_fifo_errors; /**< Transmit fifo errors */
	u64 tx_dropped;
} mac_stats_t;

typedef enum {
	MAC_SPEED_UNKNOWN,
	MAC_SPEED_10        =    10,
	MAC_SPEED_100       =   100,
	MAC_SPEED_1000      =  1000,
	MAC_SPEED_2500      =  2500,
	MAC_SPEED_5000      =  5000,
	MAC_SPEED_10000     = 10000,
} mac_speed_t;

typedef enum {
	MAC_DUPLEX_UNKNOWN,
	MAC_DUPLEX_HALF,
	MAC_DUPLEX_FULL,
} mac_duplex_t;

typedef enum {
	MAC_TYPE_UNKNOWN,
	MAC_TYPE_UNIMAC,
	MAC_TYPE_LPORT,
	MAC_TYPE_GMAC,
	MAC_TYPE_xEPON,
	MAC_TYPE_EPON_AE,
	MAC_TYPE_SF2,
	MAC_TYPE_XPORT,
	MAC_TYPE_xGPON,
	MAC_TYPE_NONBRCM,
	MAC_TYPE_MAX,
} mac_type_t;

typedef enum {
	MAC_LOOPBACK_NONE,
	MAC_LOOPBACK_LOCAL,
	MAC_LOOPBACK_REMOTE,
	MAC_LOOPBACK_BOTH,
} mac_loopback_t;

typedef struct {
	mac_speed_t speed;
	mac_duplex_t duplex;
	int flag;
#define MAC_FLAG_XGMII (1<<0)
} mac_cfg_t;

typedef struct {
	int link;
	mac_speed_t speed;
	mac_duplex_t duplex;
	int pause_rx;
	int pause_tx;
} mac_status_t;

typedef struct mac_dev_s {
	struct mac_drv_s *mac_drv;
	int mac_id;
	int enabled;
	void *priv;
	mac_stats_t stats;
	spinlock_t stats_lock;
	int stats_refresh_interval;
	int stats_refresh_cnt_dn;
} mac_dev_t;

typedef struct mac_drv_s {
	mac_type_t mac_type;
	char *name;
	int initialized;
	int (*init)(mac_dev_t *self);
	int (*read_status)(mac_dev_t *self, mac_status_t *mac_status);
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
	int (*eee_get)(mac_dev_t *self, int *enable, int *sleep_timer, int *wakeup_timer);
	int (*dev_add)(mac_dev_t *self);
	int (*dev_del)(mac_dev_t *self);
	int (*drv_init)(struct mac_drv_s *mac_drv);
	int (*loopback_set)(mac_dev_t *self, mac_loopback_t loopback_set);
	int (*loopback_get)(mac_dev_t *self, mac_loopback_t *loopback_op);
	int (*wol_enable)(mac_dev_t *mac_dev, wol_params_t *wol_params);
	int (*dt_priv)(const dt_handle_t handle, int mac_id, void **priv);
} mac_drv_t;

mac_dev_t *mac_dev_add(mac_type_t mac_type, int mac_id, void *priv);
int mac_dev_del(mac_dev_t *mac_dev);

int mac_drivers_set(void);
int mac_driver_set(mac_drv_t *mac_drv);
void mac_driver_unset(mac_type_t mac_type);
mac_dev_t *mac_dev_dt_probe(const dt_handle_t handle, u32 port_index);

static inline int mac_drv_init(mac_drv_t *mac_drv)
{
	int rc = 0;

	if (mac_drv->initialized)
		return 0;

	if (mac_drv->drv_init)
		rc = mac_drv->drv_init(mac_drv);

	if (rc == 0)
		mac_drv->initialized = 1;

	return rc;
}

static inline int mac_dev_init(mac_dev_t *mac_dev)
{
	int ret = 0;

	ret = mac_drv_init(mac_dev->mac_drv);

	if (mac_dev->mac_drv->init)
		ret |= mac_dev->mac_drv->init(mac_dev);

	return ret;
}

static inline int mac_dev_read_status(mac_dev_t *mac_dev, mac_status_t *mac_status)
{
	if (!mac_dev->mac_drv->read_status)
		return 0;

	return mac_dev->mac_drv->read_status(mac_dev, mac_status);
}

static inline int mac_dev_enable(mac_dev_t *mac_dev)
{
	mac_dev->enabled = 1;

	if (!mac_dev->mac_drv->enable)
		return 0;

	return mac_dev->mac_drv->enable(mac_dev);
}

static inline int mac_dev_disable(mac_dev_t *mac_dev)
{
	mac_dev->enabled = 0;

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

	mac_dev->stats_refresh_cnt_dn = mac_dev->stats_refresh_interval;
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

static inline int mac_dev_eee_get(mac_dev_t *mac_dev, int *enable, int *sleep_timer, int *wakeup_timer)
{
	if (!mac_dev->mac_drv->eee_get)
		return 0;

	return mac_dev->mac_drv->eee_get(mac_dev, enable, sleep_timer, wakeup_timer);
}

static inline int mac_dev_loopback_set(mac_dev_t *mac_dev, mac_loopback_t loopback_set)
{
	if (!mac_dev->mac_drv->loopback_set)
		return -1;

	return mac_dev->mac_drv->loopback_set(mac_dev, loopback_set);
}

static inline int mac_dev_loopback_get(mac_dev_t *mac_dev, mac_loopback_t *loopback_op)
{
	if (!mac_dev->mac_drv->loopback_get)
		return -1;

	return mac_dev->mac_drv->loopback_get(mac_dev, loopback_op);
}

static inline int mac_dev_wol_enable(mac_dev_t *mac_dev, wol_params_t *wol_params)
{
	if (!mac_dev->mac_drv->wol_enable)
		return 0;

	return mac_dev->mac_drv->wol_enable(mac_dev, wol_params);
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

void mac_dev_rgmii_attach(rgmii_params *params);
int mac_dev_get_stats_refresh_interval(mac_dev_t *mac_dev, int byte_cntr_width, int pkt_cntr_width, int speed);

char *mac_dev_speed_to_str(mac_speed_t speed);
char *mac_dev_speed_to_short_str(mac_speed_t speed);
#define mac_speed_2_mbps(speed)         speed
#define mac_mbps_2_speed(speed_mbps)    ((mac_speed_t) speed_mbps)

#endif

