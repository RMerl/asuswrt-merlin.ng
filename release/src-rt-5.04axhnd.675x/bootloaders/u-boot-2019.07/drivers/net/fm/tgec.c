// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2009-2011 Freescale Semiconductor, Inc.
 *	Dave Liu <daveliu@freescale.com>
 */

/* MAXFRM - maximum frame length */
#define MAXFRM_MASK	0x0000ffff

#include <common.h>
#include <phy.h>
#include <asm/types.h>
#include <asm/io.h>
#include <fsl_tgec.h>

#include "fm.h"

#define TGEC_CMD_CFG_INIT	(TGEC_CMD_CFG_NO_LEN_CHK | \
				 TGEC_CMD_CFG_RX_ER_DISC | \
				 TGEC_CMD_CFG_STAT_CLR | \
				 TGEC_CMD_CFG_PAUSE_IGNORE | \
				 TGEC_CMD_CFG_CRC_FWD)
#define TGEC_CMD_CFG_FINAL	(TGEC_CMD_CFG_NO_LEN_CHK | \
				 TGEC_CMD_CFG_RX_ER_DISC | \
				 TGEC_CMD_CFG_PAUSE_IGNORE | \
				 TGEC_CMD_CFG_CRC_FWD)

static void tgec_init_mac(struct fsl_enet_mac *mac)
{
	struct tgec *regs = mac->base;

	/* mask all interrupt */
	out_be32(&regs->imask, IMASK_MASK_ALL);

	/* clear all events */
	out_be32(&regs->ievent, IEVENT_CLEAR_ALL);

	/* set the max receive length */
	out_be32(&regs->maxfrm, mac->max_rx_len & MAXFRM_MASK);

	/*
	 * 1588 disable, insert second mac disable payload length check
	 * disable, normal operation, any rx error frame is discarded, clear
	 * counters, pause frame ignore, no promiscuous, LAN mode Rx CRC no
	 * strip, Tx CRC append, Rx disable and Tx disable
	 */
	out_be32(&regs->command_config, TGEC_CMD_CFG_INIT);
	udelay(1000);
	out_be32(&regs->command_config, TGEC_CMD_CFG_FINAL);

	/* multicast frame reception for the hash entry disable */
	out_be32(&regs->hashtable_ctrl, 0);
}

static void tgec_enable_mac(struct fsl_enet_mac *mac)
{
	struct tgec *regs = mac->base;

	setbits_be32(&regs->command_config, TGEC_CMD_CFG_RXTX_EN);
}

static void tgec_disable_mac(struct fsl_enet_mac *mac)
{
	struct tgec *regs = mac->base;

	clrbits_be32(&regs->command_config, TGEC_CMD_CFG_RXTX_EN);
}

static void tgec_set_mac_addr(struct fsl_enet_mac *mac, u8 *mac_addr)
{
	struct tgec *regs = mac->base;
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

static void tgec_set_interface_mode(struct fsl_enet_mac *mac,
					phy_interface_t type, int speed)
{
	/* nothing right now */
	return;
}

void init_tgec(struct fsl_enet_mac *mac, void *base,
		void *phyregs, int max_rx_len)
{
	mac->base = base;
	mac->phyregs = phyregs;
	mac->max_rx_len = max_rx_len;
	mac->init_mac = tgec_init_mac;
	mac->enable_mac = tgec_enable_mac;
	mac->disable_mac = tgec_disable_mac;
	mac->set_mac_addr = tgec_set_mac_addr;
	mac->set_if_mode = tgec_set_interface_mode;
}
