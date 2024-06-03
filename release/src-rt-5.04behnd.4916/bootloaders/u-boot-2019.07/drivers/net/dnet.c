/*
 * Dave Ethernet Controller driver
 *
 * Copyright (C) 2008 Dave S.r.l. <www.dave.eu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <common.h>

#ifndef CONFIG_DNET_AUTONEG_TIMEOUT
#define CONFIG_DNET_AUTONEG_TIMEOUT	5000000	/* default value */
#endif

#include <net.h>
#include <malloc.h>
#include <linux/mii.h>

#include <miiphy.h>
#include <asm/io.h>
#include <asm/unaligned.h>

#include "dnet.h"

struct dnet_device {
	struct dnet_registers	*regs;
	const struct device	*dev;
	struct eth_device	netdev;
	unsigned short		phy_addr;
};

/* get struct dnet_device from given struct netdev */
#define to_dnet(_nd) container_of(_nd, struct dnet_device, netdev)

/* function for reading internal MAC register */
u16 dnet_readw_mac(struct dnet_device *dnet, u16 reg)
{
	u16 data_read;

	/* issue a read */
	writel(reg, &dnet->regs->MACREG_ADDR);

	/* since a read/write op to the MAC is very slow,
	 * we must wait before reading the data */
	udelay(1);

	/* read data read from the MAC register */
	data_read = readl(&dnet->regs->MACREG_DATA);

	/* all done */
	return data_read;
}

/* function for writing internal MAC register */
void dnet_writew_mac(struct dnet_device *dnet, u16 reg, u16 val)
{
	/* load data to write */
	writel(val, &dnet->regs->MACREG_DATA);

	/* issue a write */
	writel(reg | DNET_INTERNAL_WRITE, &dnet->regs->MACREG_ADDR);

	/* since a read/write op to the MAC is very slow,
	 * we must wait before exiting */
	udelay(1);
}

static void dnet_mdio_write(struct dnet_device *dnet, u8 reg, u16 value)
{
	u16 tmp;

	debug(DRIVERNAME "dnet_mdio_write %02x:%02x <- %04x\n",
			dnet->phy_addr, reg, value);

	while (!(dnet_readw_mac(dnet, DNET_INTERNAL_GMII_MNG_CTL_REG) &
				DNET_INTERNAL_GMII_MNG_CMD_FIN))
		;

	/* prepare for a write operation */
	tmp = (1 << 13);

	/* only 5 bits allowed for register offset */
	reg &= 0x1f;

	/* prepare reg_value for a write */
	tmp |= (dnet->phy_addr << 8);
	tmp |= reg;

	/* write data to write first */
	dnet_writew_mac(dnet, DNET_INTERNAL_GMII_MNG_DAT_REG, value);

	/* write control word */
	dnet_writew_mac(dnet, DNET_INTERNAL_GMII_MNG_CTL_REG, tmp);

	while (!(dnet_readw_mac(dnet, DNET_INTERNAL_GMII_MNG_CTL_REG) &
				DNET_INTERNAL_GMII_MNG_CMD_FIN))
		;
}

static u16 dnet_mdio_read(struct dnet_device *dnet, u8 reg)
{
	u16 value;

	while (!(dnet_readw_mac(dnet, DNET_INTERNAL_GMII_MNG_CTL_REG) &
				DNET_INTERNAL_GMII_MNG_CMD_FIN))
		;

	/* only 5 bits allowed for register offset*/
	reg &= 0x1f;

	/* prepare reg_value for a read */
	value = (dnet->phy_addr << 8);
	value |= reg;

	/* write control word */
	dnet_writew_mac(dnet, DNET_INTERNAL_GMII_MNG_CTL_REG, value);

	/* wait for end of transfer */
	while (!(dnet_readw_mac(dnet, DNET_INTERNAL_GMII_MNG_CTL_REG) &
				DNET_INTERNAL_GMII_MNG_CMD_FIN))
		;

	value = dnet_readw_mac(dnet, DNET_INTERNAL_GMII_MNG_DAT_REG);

	debug(DRIVERNAME "dnet_mdio_read %02x:%02x <- %04x\n",
		dnet->phy_addr, reg, value);

	return value;
}

static int dnet_send(struct eth_device *netdev, void *packet, int length)
{
	struct dnet_device *dnet = to_dnet(netdev);
	int i, wrsz;
	unsigned int *bufp;
	unsigned int tx_cmd;

	debug(DRIVERNAME "[%s] Sending %u bytes\n", __func__, length);

	bufp = (unsigned int *) (((u32)packet) & 0xFFFFFFFC);
	wrsz = (u32)length + 3;
	wrsz += ((u32)packet) & 0x3;
	wrsz >>= 2;
	tx_cmd = ((((unsigned int)(packet)) & 0x03) << 16) | (u32)length;

	/* check if there is enough room for the current frame */
	if (wrsz < (DNET_FIFO_SIZE - readl(&dnet->regs->TX_FIFO_WCNT))) {
		for (i = 0; i < wrsz; i++)
			writel(*bufp++, &dnet->regs->TX_DATA_FIFO);
		/*
		 * inform MAC that a packet's written and ready
		 * to be shipped out
		 */
		writel(tx_cmd, &dnet->regs->TX_LEN_FIFO);
	} else {
		printf(DRIVERNAME "No free space (actual %d, required %d "
				"(words))\n", DNET_FIFO_SIZE -
				readl(&dnet->regs->TX_FIFO_WCNT), wrsz);
	}

	/* No one cares anyway */
	return 0;
}


static int dnet_recv(struct eth_device *netdev)
{
	struct dnet_device *dnet = to_dnet(netdev);
	unsigned int *data_ptr;
	int pkt_len, poll, i;
	u32 cmd_word;

	debug("Waiting for pkt (polling)\n");
	poll = 50;
	while ((readl(&dnet->regs->RX_FIFO_WCNT) >> 16) == 0) {
		udelay(10);  /* wait 10 usec */
		if (--poll == 0)
			return 0;	/* no pkt available */
	}

	cmd_word = readl(&dnet->regs->RX_LEN_FIFO);
	pkt_len = cmd_word & 0xFFFF;

	debug("Got pkt with size %d bytes\n", pkt_len);

	if (cmd_word & 0xDF180000)
		printf("%s packet receive error %x\n", __func__, cmd_word);

	data_ptr = (unsigned int *)net_rx_packets[0];

	for (i = 0; i < (pkt_len + 3) >> 2; i++)
		*data_ptr++ = readl(&dnet->regs->RX_DATA_FIFO);

	/* ok + 5 ?? */
	net_process_received_packet(net_rx_packets[0], pkt_len + 5);

	return 0;
}

static void dnet_set_hwaddr(struct eth_device *netdev)
{
	struct dnet_device *dnet = to_dnet(netdev);
	u16 tmp;

	tmp = get_unaligned_be16(netdev->enetaddr);
	dnet_writew_mac(dnet, DNET_INTERNAL_MAC_ADDR_0_REG, tmp);
	tmp = get_unaligned_be16(&netdev->enetaddr[2]);
	dnet_writew_mac(dnet, DNET_INTERNAL_MAC_ADDR_1_REG, tmp);
	tmp = get_unaligned_be16(&netdev->enetaddr[4]);
	dnet_writew_mac(dnet, DNET_INTERNAL_MAC_ADDR_2_REG, tmp);
}

static void dnet_phy_reset(struct dnet_device *dnet)
{
	struct eth_device *netdev = &dnet->netdev;
	int i;
	u16 status, adv;

	adv = ADVERTISE_CSMA | ADVERTISE_ALL;
	dnet_mdio_write(dnet, MII_ADVERTISE, adv);
	printf("%s: Starting autonegotiation...\n", netdev->name);
	dnet_mdio_write(dnet, MII_BMCR, (BMCR_ANENABLE
					 | BMCR_ANRESTART));

	for (i = 0; i < CONFIG_DNET_AUTONEG_TIMEOUT / 100; i++) {
		status = dnet_mdio_read(dnet, MII_BMSR);
		if (status & BMSR_ANEGCOMPLETE)
			break;
		udelay(100);
	}

	if (status & BMSR_ANEGCOMPLETE)
		printf("%s: Autonegotiation complete\n", netdev->name);
	else
		printf("%s: Autonegotiation timed out (status=0x%04x)\n",
		       netdev->name, status);
}

static int dnet_phy_init(struct dnet_device *dnet)
{
	struct eth_device *netdev = &dnet->netdev;
	u16 phy_id, status, adv, lpa;
	int media, speed, duplex;
	int i;
	u32 ctl_reg;

	/* Find a PHY */
	for (i = 0; i < 32; i++) {
		dnet->phy_addr = i;
		phy_id = dnet_mdio_read(dnet, MII_PHYSID1);
		if (phy_id != 0xffff) {
			/* ok we found it */
			printf("Found PHY at address %d PHYID (%04x:%04x)\n",
					i, phy_id,
					dnet_mdio_read(dnet, MII_PHYSID2));
			break;
		}
	}

	/* Check if the PHY is up to snuff... */
	phy_id = dnet_mdio_read(dnet, MII_PHYSID1);
	if (phy_id == 0xffff) {
		printf("%s: No PHY present\n", netdev->name);
		return -1;
	}

	status = dnet_mdio_read(dnet, MII_BMSR);
	if (!(status & BMSR_LSTATUS)) {
		/* Try to re-negotiate if we don't have link already. */
		dnet_phy_reset(dnet);

		for (i = 0; i < CONFIG_DNET_AUTONEG_TIMEOUT / 100; i++) {
			status = dnet_mdio_read(dnet, MII_BMSR);
			if (status & BMSR_LSTATUS)
				break;
			udelay(100);
		}
	}

	if (!(status & BMSR_LSTATUS)) {
		printf("%s: link down (status: 0x%04x)\n",
		       netdev->name, status);
		return -1;
	} else {
		adv = dnet_mdio_read(dnet, MII_ADVERTISE);
		lpa = dnet_mdio_read(dnet, MII_LPA);
		media = mii_nway_result(lpa & adv);
		speed = (media & (ADVERTISE_100FULL | ADVERTISE_100HALF)
			 ? 1 : 0);
		duplex = (media & ADVERTISE_FULL) ? 1 : 0;
		/* 1000BaseT ethernet is not supported */
		printf("%s: link up, %sMbps %s-duplex (lpa: 0x%04x)\n",
		       netdev->name,
		       speed ? "100" : "10",
		       duplex ? "full" : "half",
		       lpa);

		ctl_reg = dnet_readw_mac(dnet, DNET_INTERNAL_RXTX_CONTROL_REG);

		if (duplex)
			ctl_reg &= ~(DNET_INTERNAL_RXTX_CONTROL_ENABLEHALFDUP);
		else
			ctl_reg |= DNET_INTERNAL_RXTX_CONTROL_ENABLEHALFDUP;

		dnet_writew_mac(dnet, DNET_INTERNAL_RXTX_CONTROL_REG, ctl_reg);

		return 0;
	}
}

static int dnet_init(struct eth_device *netdev, bd_t *bd)
{
	struct dnet_device *dnet = to_dnet(netdev);
	u32 config;

	/*
	 * dnet_halt should have been called at some point before now,
	 * so we'll assume the controller is idle.
	 */

	/* set hardware address */
	dnet_set_hwaddr(netdev);

	if (dnet_phy_init(dnet) < 0)
		return -1;

	/* flush rx/tx fifos */
	writel(DNET_SYS_CTL_RXFIFOFLUSH | DNET_SYS_CTL_TXFIFOFLUSH,
			&dnet->regs->SYS_CTL);
	udelay(1000);
	writel(0, &dnet->regs->SYS_CTL);

	config = dnet_readw_mac(dnet, DNET_INTERNAL_RXTX_CONTROL_REG);

	config |= DNET_INTERNAL_RXTX_CONTROL_RXPAUSE |
			DNET_INTERNAL_RXTX_CONTROL_RXBROADCAST |
			DNET_INTERNAL_RXTX_CONTROL_DROPCONTROL |
			DNET_INTERNAL_RXTX_CONTROL_DISCFXFCS;

	dnet_writew_mac(dnet, DNET_INTERNAL_RXTX_CONTROL_REG, config);

	/* Enable TX and RX */
	dnet_writew_mac(dnet, DNET_INTERNAL_MODE_REG,
			DNET_INTERNAL_MODE_RXEN | DNET_INTERNAL_MODE_TXEN);

	return 0;
}

static void dnet_halt(struct eth_device *netdev)
{
	struct dnet_device *dnet = to_dnet(netdev);

	/* Disable TX and RX */
	dnet_writew_mac(dnet, DNET_INTERNAL_MODE_REG, 0);
}

int dnet_eth_initialize(int id, void *regs, unsigned int phy_addr)
{
	struct dnet_device *dnet;
	struct eth_device *netdev;
	unsigned int dev_capa;

	dnet = malloc(sizeof(struct dnet_device));
	if (!dnet) {
		printf("Error: Failed to allocate memory for DNET%d\n", id);
		return -1;
	}
	memset(dnet, 0, sizeof(struct dnet_device));

	netdev = &dnet->netdev;

	dnet->regs = (struct dnet_registers *)regs;
	dnet->phy_addr = phy_addr;

	sprintf(netdev->name, "dnet%d", id);
	netdev->init = dnet_init;
	netdev->halt = dnet_halt;
	netdev->send = dnet_send;
	netdev->recv = dnet_recv;

	dev_capa = readl(&dnet->regs->VERCAPS) & 0xFFFF;
	debug("%s: has %smdio, %sirq, %sgigabit, %sdma \n", netdev->name,
		(dev_capa & DNET_HAS_MDIO) ? "" : "no ",
		(dev_capa & DNET_HAS_IRQ) ? "" : "no ",
		(dev_capa & DNET_HAS_GIGABIT) ? "" : "no ",
		(dev_capa & DNET_HAS_DMA) ? "" : "no ");

	eth_register(netdev);

	return 0;
}
