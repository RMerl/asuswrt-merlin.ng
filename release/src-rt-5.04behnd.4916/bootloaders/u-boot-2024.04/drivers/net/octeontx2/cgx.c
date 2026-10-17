// SPDX-License-Identifier:    GPL-2.0
/*
 * Copyright (C) 2018 Marvell International Ltd.
 */

#include <dm.h>
#include <errno.h>
#include <malloc.h>
#include <misc.h>
#include <net.h>
#include <pci_ids.h>
#include <linux/list.h>
#include <asm/arch/board.h>
#include <asm/arch/csrs/csrs-cgx.h>
#include <asm/io.h>

#include "cgx.h"

char lmac_type_to_str[][8] = {
	"SGMII",
	"XAUI",
	"RXAUI",
	"10G_R",
	"40G_R",
	"RGMII",
	"QSGMII",
	"25G_R",
	"50G_R",
	"100G_R",
	"USXGMII",
};

char lmac_speed_to_str[][8] = {
	"0",
	"10M",
	"100M",
	"1G",
	"2.5G",
	"5G",
	"10G",
	"20G",
	"25G",
	"40G",
	"50G",
	"80G",
	"100G",
};

/**
 * Given an LMAC/PF instance number, return the lmac
 * Per design, each PF has only one LMAC mapped.
 *
 * @param instance	instance to find
 *
 * Return:	pointer to lmac data structure or NULL if not found
 */
struct lmac *nix_get_cgx_lmac(int lmac_instance)
{
	struct cgx *cgx;
	struct udevice *dev;
	int i, idx, err;

	for (i = 0; i < CGX_PER_NODE; i++) {
		err = dm_pci_find_device(PCI_VENDOR_ID_CAVIUM,
					 PCI_DEVICE_ID_OCTEONTX2_CGX, i,
					 &dev);
		if (err)
			continue;

		cgx = dev_get_priv(dev);
		debug("%s udev %p cgx %p instance %d\n", __func__, dev, cgx,
		      lmac_instance);
		for (idx = 0; idx < cgx->lmac_count; idx++) {
			if (cgx->lmac[idx]->instance == lmac_instance)
				return cgx->lmac[idx];
		}
	}
	return NULL;
}

void cgx_lmac_mac_filter_clear(struct lmac *lmac)
{
	union cgxx_cmrx_rx_dmac_ctl0 dmac_ctl0;
	union cgxx_cmr_rx_dmacx_cam0 dmac_cam0;
	void *reg_addr;

	dmac_cam0.u = 0x0;
	reg_addr = lmac->cgx->reg_base +
			CGXX_CMR_RX_DMACX_CAM0(lmac->lmac_id * 8);
	writeq(dmac_cam0.u, reg_addr);
	debug("%s: reg %p dmac_cam0 %llx\n", __func__, reg_addr, dmac_cam0.u);

	dmac_ctl0.u = 0x0;
	dmac_ctl0.s.bcst_accept = 1;
	dmac_ctl0.s.mcst_mode = 1;
	dmac_ctl0.s.cam_accept = 0;
	reg_addr = lmac->cgx->reg_base +
			CGXX_CMRX_RX_DMAC_CTL0(lmac->lmac_id);
	writeq(dmac_ctl0.u, reg_addr);
	debug("%s: reg %p dmac_ctl0 %llx\n", __func__, reg_addr, dmac_ctl0.u);
}

void cgx_lmac_mac_filter_setup(struct lmac *lmac)
{
	union cgxx_cmrx_rx_dmac_ctl0 dmac_ctl0;
	union cgxx_cmr_rx_dmacx_cam0 dmac_cam0;
	u64 mac, tmp;
	void *reg_addr;

	memcpy((void *)&tmp, lmac->mac_addr, 6);
	debug("%s: tmp %llx\n", __func__, tmp);
	debug("%s: swab tmp %llx\n", __func__, swab64(tmp));
	mac = swab64(tmp) >> 16;
	debug("%s: mac %llx\n", __func__, mac);
	dmac_cam0.u = 0x0;
	dmac_cam0.s.id = lmac->lmac_id;
	dmac_cam0.s.adr = mac;
	dmac_cam0.s.en = 1;
	reg_addr = lmac->cgx->reg_base +
			CGXX_CMR_RX_DMACX_CAM0(lmac->lmac_id * 8);
	writeq(dmac_cam0.u, reg_addr);
	debug("%s: reg %p dmac_cam0 %llx\n", __func__, reg_addr, dmac_cam0.u);
	dmac_ctl0.u = 0x0;
	dmac_ctl0.s.bcst_accept = 1;
	dmac_ctl0.s.mcst_mode = 0;
	dmac_ctl0.s.cam_accept = 1;
	reg_addr = lmac->cgx->reg_base +
			CGXX_CMRX_RX_DMAC_CTL0(lmac->lmac_id);
	writeq(dmac_ctl0.u, reg_addr);
	debug("%s: reg %p dmac_ctl0 %llx\n", __func__, reg_addr, dmac_ctl0.u);
}

int cgx_lmac_set_pkind(struct lmac *lmac, u8 lmac_id, int pkind)
{
	cgx_write(lmac->cgx, lmac_id, CGXX_CMRX_RX_ID_MAP(0),
		  (pkind & 0x3f));
	return 0;
}

int cgx_lmac_link_status(struct lmac *lmac, int lmac_id, u64 *status)
{
	int ret = 0;

	ret = cgx_intf_get_link_sts(lmac->cgx->cgx_id, lmac_id, status);
	if (ret) {
		debug("%s request failed for cgx%d lmac%d\n",
		      __func__, lmac->cgx->cgx_id, lmac->lmac_id);
		ret = -1;
	}
	return ret;
}

int cgx_lmac_rx_tx_enable(struct lmac *lmac, int lmac_id, bool enable)
{
	struct cgx *cgx = lmac->cgx;
	union cgxx_cmrx_config cmrx_config;

	if (!cgx || lmac_id >= cgx->lmac_count)
		return -ENODEV;

	cmrx_config.u = cgx_read(cgx, lmac_id, CGXX_CMRX_CONFIG(0));
	cmrx_config.s.data_pkt_rx_en =
	cmrx_config.s.data_pkt_tx_en = enable ? 1 : 0;
	cgx_write(cgx, lmac_id, CGXX_CMRX_CONFIG(0), cmrx_config.u);
	return 0;
}

int cgx_lmac_link_enable(struct lmac *lmac, int lmac_id, bool enable,
			 u64 *status)
{
	int ret = 0;

	ret = cgx_intf_link_up_dwn(lmac->cgx->cgx_id, lmac_id, enable,
				   status);
	if (ret) {
		debug("%s request failed for cgx%d lmac%d\n",
		      __func__, lmac->cgx->cgx_id, lmac->lmac_id);
		ret = -1;
	}
	return ret;
}

int cgx_lmac_internal_loopback(struct lmac *lmac, int lmac_id, bool enable)
{
	struct cgx *cgx = lmac->cgx;
	union cgxx_cmrx_config cmrx_cfg;
	union cgxx_gmp_pcs_mrx_control mrx_control;
	union cgxx_spux_control1 spux_control1;
	enum lmac_type lmac_type;

	if (!cgx || lmac_id >= cgx->lmac_count)
		return -ENODEV;

	cmrx_cfg.u = cgx_read(cgx, lmac_id, CGXX_CMRX_CONFIG(0));
	lmac_type = cmrx_cfg.s.lmac_type;
	if (lmac_type == LMAC_MODE_SGMII || lmac_type == LMAC_MODE_QSGMII) {
		mrx_control.u = cgx_read(cgx, lmac_id,
					 CGXX_GMP_PCS_MRX_CONTROL(0));
		mrx_control.s.loopbck1 = enable ? 1 : 0;
		cgx_write(cgx, lmac_id, CGXX_GMP_PCS_MRX_CONTROL(0),
			  mrx_control.u);
	} else {
		spux_control1.u = cgx_read(cgx, lmac_id,
					   CGXX_SPUX_CONTROL1(0));
		spux_control1.s.loopbck = enable ? 1 : 0;
		cgx_write(cgx, lmac_id, CGXX_SPUX_CONTROL1(0),
			  spux_control1.u);
	}
	return 0;
}

static int cgx_lmac_init(struct cgx *cgx)
{
	struct lmac *lmac;
	union cgxx_cmrx_config cmrx_cfg;
	static int instance = 1;
	int i;

	cgx->lmac_count = cgx_read(cgx, 0, CGXX_CMR_RX_LMACS());
	debug("%s: Found %d lmacs for cgx %d@%p\n", __func__, cgx->lmac_count,
	      cgx->cgx_id, cgx->reg_base);

	for (i = 0; i < cgx->lmac_count; i++) {
		lmac = calloc(1, sizeof(*lmac));
		if (!lmac)
			return -ENOMEM;
		lmac->instance = instance++;
		snprintf(lmac->name, sizeof(lmac->name), "cgx_fwi_%d_%d",
			 cgx->cgx_id, i);
		/* Get LMAC type */
		cmrx_cfg.u = cgx_read(cgx, i, CGXX_CMRX_CONFIG(0));
		lmac->lmac_type = cmrx_cfg.s.lmac_type;

		lmac->lmac_id = i;
		lmac->cgx = cgx;
		cgx->lmac[i] = lmac;
		debug("%s: map id %d to lmac %p (%s), type:%d instance %d\n",
		      __func__, i, lmac, lmac->name, lmac->lmac_type,
		      lmac->instance);
		lmac->init_pend = 1;
		printf("CGX%d LMAC%d [%s]\n", lmac->cgx->cgx_id,
		       lmac->lmac_id, lmac_type_to_str[lmac->lmac_type]);
		octeontx2_board_get_mac_addr((lmac->instance - 1),
					     lmac->mac_addr);
		debug("%s: MAC %pM\n", __func__, lmac->mac_addr);
		cgx_lmac_mac_filter_setup(lmac);
	}
	return 0;
}

int cgx_probe(struct udevice *dev)
{
	struct cgx *cgx = dev_get_priv(dev);
	int err;

	cgx->reg_base = dm_pci_map_bar(dev, PCI_BASE_ADDRESS_0, 0, 0, PCI_REGION_TYPE,
				       PCI_REGION_MEM);
	cgx->dev = dev;
	cgx->cgx_id = ((u64)(cgx->reg_base) >> 24) & 0x7;

	debug("%s CGX BAR %p, id: %d\n", __func__, cgx->reg_base,
	      cgx->cgx_id);
	debug("%s CGX %p, udev: %p\n", __func__, cgx, dev);

	err = cgx_lmac_init(cgx);

	return err;
}

int cgx_remove(struct udevice *dev)
{
	struct cgx *cgx = dev_get_priv(dev);
	int i;

	debug("%s: cgx remove reg_base %p cgx_id %d",
	      __func__, cgx->reg_base, cgx->cgx_id);
	for (i = 0; i < cgx->lmac_count; i++)
		cgx_lmac_mac_filter_clear(cgx->lmac[i]);

	return 0;
}

U_BOOT_DRIVER(cgx) = {
	.name	= "cgx",
	.id	= UCLASS_MISC,
	.probe	= cgx_probe,
	.remove	= cgx_remove,
	.priv_auto	= sizeof(struct cgx),
};

static struct pci_device_id cgx_supported[] = {
	{PCI_VDEVICE(CAVIUM, PCI_DEVICE_ID_CAVIUM_CGX) },
	{}
};

U_BOOT_PCI_DEVICE(cgx, cgx_supported);
