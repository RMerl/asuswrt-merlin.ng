// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2012 Freescale Semiconductor, Inc.
 *	Roy Zang <tie-fei.zang@freescale.com>
 */

/* MAXFRM - maximum frame length */
#define MAXFRM_MASK	0x0000ffff

#include <common.h>
#include <phy.h>
#include <asm/types.h>
#include <asm/io.h>
#include <fsl_memac.h>

#include "fm.h"

static void memac_init_mac(struct fsl_enet_mac *mac)
{
	struct memac *regs = mac->base;

	/* mask all interrupt */
	out_be32(&regs->imask, IMASK_MASK_ALL);

	/* clear all events */
	out_be32(&regs->ievent, IEVENT_CLEAR_ALL);

	/* set the max receive length */
	out_be32(&regs->maxfrm, mac->max_rx_len & MAXFRM_MASK);

	/* multicast frame reception for the hash entry disable */
	out_be32(&regs->hashtable_ctrl, 0);
}

static void memac_enable_mac(struct fsl_enet_mac *mac)
{
	struct memac *regs = mac->base;

	setbits_be32(&regs->command_config,
		     MEMAC_CMD_CFG_RXTX_EN | MEMAC_CMD_CFG_NO_LEN_CHK);
}

static void memac_disable_mac(struct fsl_enet_mac *mac)
{
	struct memac *regs = mac->base;

	clrbits_be32(&regs->command_config, MEMAC_CMD_CFG_RXTX_EN);
}

static void memac_set_mac_addr(struct fsl_enet_mac *mac, u8 *mac_addr)
{
	struct memac *regs = mac->base;
	u32 mac_addr0, mac_addr1;

	/*
	 * if a station address of 0x12345678ABCD, perform a write to
	 * MAC_ADDR0 of 0x78563412, MAC_ADDR1 of 0x0000CDAB
	 */
	mac_addr0 = (mac_addr[3] << 24) | (mac_addr[2] << 16) | \
			(mac_addr[1] << 8)  | (mac_addr[0]);
	out_be32(&regs->mac_addr_0, mac_addr0);

	mac_addr1 = ((mac_addr[5] << 8) | mac_addr[4]) & 0x0000ffff;
	out_be32(&regs->mac_addr_1, mac_addr1);
}

static void memac_set_interface_mode(struct fsl_enet_mac *mac,
					phy_interface_t type, int speed)
{
	/* Roy need more work here */

	struct memac *regs = mac->base;
	u32 if_mode, if_status;

	/* clear all bits relative with interface mode */
	if_mode = in_be32(&regs->if_mode);
	if_status = in_be32(&regs->if_status);

	/* set interface mode */
	switch (type) {
	case PHY_INTERFACE_MODE_GMII:
		if_mode &= ~IF_MODE_MASK;
		if_mode |= IF_MODE_GMII;
		break;
	case PHY_INTERFACE_MODE_RGMII:
	case PHY_INTERFACE_MODE_RGMII_TXID:
		if_mode |= (IF_MODE_GMII | IF_MODE_RG);
		break;
	case PHY_INTERFACE_MODE_RMII:
		if_mode |= (IF_MODE_GMII | IF_MODE_RM);
		break;
	case PHY_INTERFACE_MODE_SGMII:
	case PHY_INTERFACE_MODE_SGMII_2500:
	case PHY_INTERFACE_MODE_QSGMII:
		if_mode &= ~IF_MODE_MASK;
		if_mode |= (IF_MODE_GMII);
		break;
	case PHY_INTERFACE_MODE_XGMII:
		if_mode &= ~IF_MODE_MASK;
		if_mode |= IF_MODE_XGMII;
		break;
	default:
		break;
	}
	/* Enable automatic speed selection for Non-XGMII */
	if (type != PHY_INTERFACE_MODE_XGMII)
		if_mode |= IF_MODE_EN_AUTO;

	if (type == PHY_INTERFACE_MODE_RGMII ||
	    type == PHY_INTERFACE_MODE_RGMII_TXID) {
		if_mode &= ~IF_MODE_EN_AUTO;
		if_mode &= ~IF_MODE_SETSP_MASK;
		switch (speed) {
		case SPEED_1000:
			if_mode |= IF_MODE_SETSP_1000M;
			break;
		case SPEED_100:
			if_mode |= IF_MODE_SETSP_100M;
			break;
		case SPEED_10:
			if_mode |= IF_MODE_SETSP_10M;
		default:
			break;
		}
	}

	debug(" %s, if_mode = %x\n", __func__,  if_mode);
	debug(" %s, if_status = %x\n", __func__,  if_status);
	out_be32(&regs->if_mode, if_mode);
	return;
}

void init_memac(struct fsl_enet_mac *mac, void *base,
		void *phyregs, int max_rx_len)
{
	mac->base = base;
	mac->phyregs = phyregs;
	mac->max_rx_len = max_rx_len;
	mac->init_mac = memac_init_mac;
	mac->enable_mac = memac_enable_mac;
	mac->disable_mac = memac_disable_mac;
	mac->set_mac_addr = memac_set_mac_addr;
	mac->set_if_mode = memac_set_interface_mode;
}
