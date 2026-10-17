/* SPDX-License-Identifier:    GPL-2.0
 *
 * Copyright (C) 2018 Marvell International Ltd.
 */

#ifndef __CGX_H__
#define __CGX_H__

#include "cgx_intf.h"

#define PCI_DEVICE_ID_OCTEONTX2_CGX	0xA059

#define MAX_LMAC_PER_CGX		4
#define CGX_PER_NODE			3

enum lmac_type {
	LMAC_MODE_SGMII		= 0,
	LMAC_MODE_XAUI		= 1,
	LMAC_MODE_RXAUI		= 2,
	LMAC_MODE_10G_R		= 3,
	LMAC_MODE_40G_R		= 4,
	LMAC_MODE_QSGMII	= 6,
	LMAC_MODE_25G_R		= 7,
	LMAC_MODE_50G_R		= 8,
	LMAC_MODE_100G_R	= 9,
	LMAC_MODE_USXGMII	= 10,
};

extern char lmac_type_to_str[][8];

extern char lmac_speed_to_str[][8];

struct lmac_priv {
	u8 enable:1;
	u8 full_duplex:1;
	u8 speed:4;
	u8 mode:1;
	u8 rsvd:1;
	u8 mac_addr[6];
};

struct cgx;
struct nix;
struct nix_af;

struct lmac {
	struct cgx	*cgx;
	struct nix	*nix;
	char		name[16];
	enum lmac_type	lmac_type;
	bool		init_pend;
	u8		instance;
	u8		lmac_id;
	u8		pknd;
	u8		link_num;
	u32		chan_num;
	u8		mac_addr[6];
};

struct cgx {
	struct nix_af		*nix_af;
	void __iomem		*reg_base;
	struct udevice		*dev;
	struct lmac		*lmac[MAX_LMAC_PER_CGX];
	u8			cgx_id;
	u8			lmac_count;
};

static inline void cgx_write(struct cgx *cgx, u8 lmac, u64 offset, u64 val)
{
	writeq(val, cgx->reg_base + CMR_SHIFT(lmac) + offset);
}

static inline u64 cgx_read(struct cgx *cgx, u8 lmac, u64 offset)
{
	return readq(cgx->reg_base + CMR_SHIFT(lmac) + offset);
}

/**
 * Given an LMAC/PF instance number, return the lmac
 * Per design, each PF has only one LMAC mapped.
 *
 * @param instance	instance to find
 *
 * Return:	pointer to lmac data structure or NULL if not found
 */
struct lmac *nix_get_cgx_lmac(int lmac_instance);

int cgx_lmac_set_pkind(struct lmac *lmac, u8 lmac_id, int pkind);
int cgx_lmac_internal_loopback(struct lmac *lmac, int lmac_id, bool enable);
int cgx_lmac_rx_tx_enable(struct lmac *lmac, int lmac_id, bool enable);
int cgx_lmac_link_enable(struct lmac *lmac, int lmac_id, bool enable,
			 u64 *status);
int cgx_lmac_link_status(struct lmac *lmac, int lmac_id, u64 *status);
void cgx_lmac_mac_filter_setup(struct lmac *lmac);

int cgx_intf_get_link_sts(u8 cgx, u8 lmac, u64 *lnk_sts);
int cgx_intf_link_up_dwn(u8 cgx, u8 lmac, u8 up_dwn, u64 *lnk_sts);
int cgx_intf_get_mac_addr(u8 cgx, u8 lmac, u8 *mac);
int cgx_intf_set_macaddr(struct udevice *dev);
int cgx_intf_prbs(u8 qlm, u8 mode, u32 time, u8 lane);
int cgx_intf_display_eye(u8 qlm, u8 lane);
int cgx_intf_display_serdes(u8 qlm, u8 lane);

#endif /* __CGX_H__ */
