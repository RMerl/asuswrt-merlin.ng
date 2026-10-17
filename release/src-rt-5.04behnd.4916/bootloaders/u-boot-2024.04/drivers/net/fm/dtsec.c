// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2009-2011 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <asm/types.h>
#include <asm/io.h>
#include <fsl_dtsec.h>
#include <fsl_mdio.h>
#include <phy.h>
#include <linux/delay.h>

#include "fm.h"

#define RCTRL_INIT	(RCTRL_GRS | RCTRL_UPROM)
#define TCTRL_INIT	TCTRL_GTS
#define MACCFG1_INIT	MACCFG1_SOFT_RST

#define MACCFG2_INIT	(MACCFG2_PRE_LEN(0x7) | MACCFG2_LEN_CHECK | \
			 MACCFG2_PAD_CRC | MACCFG2_FULL_DUPLEX | \
			 MACCFG2_IF_MODE_NIBBLE)

/* MAXFRM - maximum frame length register */
#define MAXFRM_MASK		0x00003fff

static void dtsec_init_mac(struct fsl_enet_mac *mac)
{
	struct dtsec *regs = mac->base;

	/* soft reset */
	out_be32(&regs->maccfg1, MACCFG1_SOFT_RST);
	udelay(1000);

	/* clear soft reset, Rx/Tx MAC disable */
	out_be32(&regs->maccfg1, 0);

	/* graceful stop rx */
	out_be32(&regs->rctrl, RCTRL_INIT);
	udelay(1000);

	/* graceful stop tx */
	out_be32(&regs->tctrl, TCTRL_INIT);
	udelay(1000);

	/* disable all interrupts */
	out_be32(&regs->imask, IMASK_MASK_ALL);

	/* clear all events */
	out_be32(&regs->ievent, IEVENT_CLEAR_ALL);

	/* set the max Rx length */
	out_be32(&regs->maxfrm, mac->max_rx_len & MAXFRM_MASK);

	/* set the ecntrl to reset value */
	out_be32(&regs->ecntrl, ECNTRL_DEFAULT);

	/*
	 * Rx length check, no strip CRC for Rx, pad and append CRC for Tx,
	 * full duplex
	 */
	out_be32(&regs->maccfg2, MACCFG2_INIT);
}

static void dtsec_enable_mac(struct fsl_enet_mac *mac)
{
	struct dtsec *regs = mac->base;

	/* enable Rx/Tx MAC */
	setbits_be32(&regs->maccfg1, MACCFG1_RXTX_EN);

	/* clear the graceful Rx stop */
	clrbits_be32(&regs->rctrl, RCTRL_GRS);

	/* clear the graceful Tx stop */
	clrbits_be32(&regs->tctrl, TCTRL_GTS);
}

static void dtsec_disable_mac(struct fsl_enet_mac *mac)
{
	struct dtsec *regs = mac->base;

	/* graceful Rx stop */
	setbits_be32(&regs->rctrl, RCTRL_GRS);

	/* graceful Tx stop */
	setbits_be32(&regs->tctrl, TCTRL_GTS);

	/* disable Rx/Tx MAC */
	clrbits_be32(&regs->maccfg1, MACCFG1_RXTX_EN);
}

static void dtsec_set_mac_addr(struct fsl_enet_mac *mac, u8 *mac_addr)
{
	struct dtsec *regs = mac->base;
	u32 mac_addr1, mac_addr2;

	/*
	 * if a station address of 0x12345678ABCD, perform a write to
	 * MACSTNADDR1 of 0xCDAB7856, MACSTNADDR2 of 0x34120000
	 */
	mac_addr1 = (mac_addr[5] << 24) | (mac_addr[4] << 16) | \
			(mac_addr[3] << 8)  | (mac_addr[2]);
	out_be32(&regs->macstnaddr1, mac_addr1);

	mac_addr2 = ((mac_addr[1] << 24) | (mac_addr[0] << 16)) & 0xffff0000;
	out_be32(&regs->macstnaddr2, mac_addr2);
}

static void dtsec_set_interface_mode(struct fsl_enet_mac *mac,
		phy_interface_t type, int speed)
{
	struct dtsec *regs = mac->base;
	u32 ecntrl, maccfg2;

	/* clear all bits relative with interface mode */
	ecntrl = in_be32(&regs->ecntrl);
	ecntrl &= ~(ECNTRL_TBIM | ECNTRL_GMIIM | ECNTRL_RPM |
				  ECNTRL_R100M | ECNTRL_SGMIIM);

	maccfg2 = in_be32(&regs->maccfg2);
	maccfg2 &= ~MACCFG2_IF_MODE_MASK;

	if (speed == SPEED_1000)
		maccfg2 |= MACCFG2_IF_MODE_BYTE;
	else
		maccfg2 |= MACCFG2_IF_MODE_NIBBLE;

	/* set interface mode */
	switch (type) {
	case PHY_INTERFACE_MODE_GMII:
		ecntrl |= ECNTRL_GMIIM;
		break;
	case PHY_INTERFACE_MODE_RGMII:
		ecntrl |= (ECNTRL_GMIIM | ECNTRL_RPM);
		if (speed == SPEED_100)
			ecntrl |= ECNTRL_R100M;
		break;
	case PHY_INTERFACE_MODE_RMII:
		if (speed == SPEED_100)
			ecntrl |= ECNTRL_R100M;
		break;
	case PHY_INTERFACE_MODE_SGMII:
		ecntrl |= (ECNTRL_SGMIIM | ECNTRL_TBIM);
		if (speed == SPEED_100)
			ecntrl |= ECNTRL_R100M;
		break;
	default:
		break;
	}

	out_be32(&regs->ecntrl, ecntrl);
	out_be32(&regs->maccfg2, maccfg2);
}

void init_dtsec(struct fsl_enet_mac *mac, void *base,
		void *phyregs, int max_rx_len)
{
	mac->base = base;
	mac->phyregs = phyregs;
	mac->max_rx_len = max_rx_len;
	mac->init_mac = dtsec_init_mac;
	mac->enable_mac = dtsec_enable_mac;
	mac->disable_mac = dtsec_disable_mac;
	mac->set_mac_addr = dtsec_set_mac_addr;
	mac->set_if_mode = dtsec_set_interface_mode;
}
