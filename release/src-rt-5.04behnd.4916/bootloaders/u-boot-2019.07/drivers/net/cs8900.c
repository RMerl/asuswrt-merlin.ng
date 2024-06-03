// SPDX-License-Identifier: GPL-2.0+
/*
 * Cirrus Logic CS8900A Ethernet
 *
 * (C) 2009 Ben Warren , biggerbadderben@gmail.com
 *     Converted to use CONFIG_NET_MULTI API
 *
 * (C) 2003 Wolfgang Denk, wd@denx.de
 *     Extension to synchronize ethaddr environment variable
 *     against value in EEPROM
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * Copyright (C) 1999 Ben Williamson <benw@pobox.com>
 *
 * This program is loaded into SRAM in bootstrap mode, where it waits
 * for commands on UART1 to read and write memory, jump to code etc.
 * A design goal for this program is to be entirely independent of the
 * target board.  Anything with a CL-PS7111 or EP7211 should be able to run
 * this code in bootstrap mode.  All the board specifics can be handled on
 * the host.
 */

#include <common.h>
#include <command.h>
#include <asm/io.h>
#include <net.h>
#include <malloc.h>
#include "cs8900.h"

#undef DEBUG

/* packet page register access functions */

#ifdef CONFIG_CS8900_BUS32

#define REG_WRITE(v, a) writel((v),(a))
#define REG_READ(a) readl((a))

/* we don't need 16 bit initialisation on 32 bit bus */
#define get_reg_init_bus(r,d) get_reg((r),(d))

#else

#define REG_WRITE(v, a) writew((v),(a))
#define REG_READ(a) readw((a))

static u16 get_reg_init_bus(struct eth_device *dev, int regno)
{
	/* force 16 bit busmode */
	struct cs8900_priv *priv = (struct cs8900_priv *)(dev->priv);
	uint8_t volatile * const iob = (uint8_t volatile * const)dev->iobase;

	readb(iob);
	readb(iob + 1);
	readb(iob);
	readb(iob + 1);
	readb(iob);

	REG_WRITE(regno, &priv->regs->pptr);
	return REG_READ(&priv->regs->pdata);
}
#endif

static u16 get_reg(struct eth_device *dev, int regno)
{
	struct cs8900_priv *priv = (struct cs8900_priv *)(dev->priv);
	REG_WRITE(regno, &priv->regs->pptr);
	return REG_READ(&priv->regs->pdata);
}


static void put_reg(struct eth_device *dev, int regno, u16 val)
{
	struct cs8900_priv *priv = (struct cs8900_priv *)(dev->priv);
	REG_WRITE(regno, &priv->regs->pptr);
	REG_WRITE(val, &priv->regs->pdata);
}

static void cs8900_reset(struct eth_device *dev)
{
	int tmo;
	u16 us;

	/* reset NIC */
	put_reg(dev, PP_SelfCTL, get_reg(dev, PP_SelfCTL) | PP_SelfCTL_Reset);

	/* wait for 200ms */
	udelay(200000);
	/* Wait until the chip is reset */

	tmo = get_timer(0) + 1 * CONFIG_SYS_HZ;
	while ((((us = get_reg_init_bus(dev, PP_SelfSTAT)) &
		PP_SelfSTAT_InitD) == 0) && tmo < get_timer(0))
		/*NOP*/;
}

static void cs8900_reginit(struct eth_device *dev)
{
	/* receive only error free packets addressed to this card */
	put_reg(dev, PP_RxCTL,
		PP_RxCTL_IA | PP_RxCTL_Broadcast | PP_RxCTL_RxOK);
	/* do not generate any interrupts on receive operations */
	put_reg(dev, PP_RxCFG, 0);
	/* do not generate any interrupts on transmit operations */
	put_reg(dev, PP_TxCFG, 0);
	/* do not generate any interrupts on buffer operations */
	put_reg(dev, PP_BufCFG, 0);
	/* enable transmitter/receiver mode */
	put_reg(dev, PP_LineCTL, PP_LineCTL_Rx | PP_LineCTL_Tx);
}

void cs8900_get_enetaddr(struct eth_device *dev)
{
	int i;

	/* verify chip id */
	if (get_reg_init_bus(dev, PP_ChipID) != 0x630e)
		return;
	cs8900_reset(dev);
	if ((get_reg(dev, PP_SelfSTAT) &
		(PP_SelfSTAT_EEPROM | PP_SelfSTAT_EEPROM_OK)) ==
		(PP_SelfSTAT_EEPROM | PP_SelfSTAT_EEPROM_OK)) {

		/* Load the MAC from EEPROM */
		for (i = 0; i < 3; i++) {
			u32 Addr;

			Addr = get_reg(dev, PP_IA + i * 2);
			dev->enetaddr[i * 2] = Addr & 0xFF;
			dev->enetaddr[i * 2 + 1] = Addr >> 8;
		}
	}
}

void cs8900_halt(struct eth_device *dev)
{
	/* disable transmitter/receiver mode */
	put_reg(dev, PP_LineCTL, 0);

	/* "shutdown" to show ChipID or kernel wouldn't find he cs8900 ... */
	get_reg_init_bus(dev, PP_ChipID);
}

static int cs8900_init(struct eth_device *dev, bd_t * bd)
{
	uchar *enetaddr = dev->enetaddr;
	u16 id;

	/* verify chip id */
	id = get_reg_init_bus(dev, PP_ChipID);
	if (id != 0x630e) {
		printf ("CS8900 Ethernet chip not found: "
			"ID=0x%04x instead 0x%04x\n", id, 0x630e);
		return 1;
	}

	cs8900_reset (dev);
	/* set the ethernet address */
	put_reg(dev, PP_IA + 0, enetaddr[0] | (enetaddr[1] << 8));
	put_reg(dev, PP_IA + 2, enetaddr[2] | (enetaddr[3] << 8));
	put_reg(dev, PP_IA + 4, enetaddr[4] | (enetaddr[5] << 8));

	cs8900_reginit(dev);
	return 0;
}

/* Get a data block via Ethernet */
static int cs8900_recv(struct eth_device *dev)
{
	int i;
	u16 rxlen;
	u16 *addr;
	u16 status;

	struct cs8900_priv *priv = (struct cs8900_priv *)(dev->priv);

	status = get_reg(dev, PP_RER);

	if ((status & PP_RER_RxOK) == 0)
		return 0;

	status = REG_READ(&priv->regs->rtdata);
	rxlen = REG_READ(&priv->regs->rtdata);

	if (rxlen > PKTSIZE_ALIGN + PKTALIGN)
		debug("packet too big!\n");
	for (addr = (u16 *)net_rx_packets[0], i = rxlen >> 1; i > 0; i--)
		*addr++ = REG_READ(&priv->regs->rtdata);
	if (rxlen & 1)
		*addr++ = REG_READ(&priv->regs->rtdata);

	/* Pass the packet up to the protocol layers. */
	net_process_received_packet(net_rx_packets[0], rxlen);
	return rxlen;
}

/* Send a data block via Ethernet. */
static int cs8900_send(struct eth_device *dev, void *packet, int length)
{
	volatile u16 *addr;
	int tmo;
	u16 s;
	struct cs8900_priv *priv = (struct cs8900_priv *)(dev->priv);

retry:
	/* initiate a transmit sequence */
	REG_WRITE(PP_TxCmd_TxStart_Full, &priv->regs->txcmd);
	REG_WRITE(length, &priv->regs->txlen);

	/* Test to see if the chip has allocated memory for the packet */
	if ((get_reg(dev, PP_BusSTAT) & PP_BusSTAT_TxRDY) == 0) {
		/* Oops... this should not happen! */
		debug("cs: unable to send packet; retrying...\n");
		for (tmo = get_timer(0) + 5 * CONFIG_SYS_HZ;
			get_timer(0) < tmo;)
			/*NOP*/;
		cs8900_reset(dev);
		cs8900_reginit(dev);
		goto retry;
	}

	/* Write the contents of the packet */
	/* assume even number of bytes */
	for (addr = packet; length > 0; length -= 2)
		REG_WRITE(*addr++, &priv->regs->rtdata);

	/* wait for transfer to succeed */
	tmo = get_timer(0) + 5 * CONFIG_SYS_HZ;
	while ((s = get_reg(dev, PP_TER) & ~0x1F) == 0) {
		if (get_timer(0) >= tmo)
			break;
	}

	/* nothing */ ;
	if((s & (PP_TER_CRS | PP_TER_TxOK)) != PP_TER_TxOK) {
		debug("\ntransmission error %#x\n", s);
	}

	return 0;
}

static void cs8900_e2prom_ready(struct eth_device *dev)
{
	while (get_reg(dev, PP_SelfSTAT) & SI_BUSY)
		;
}

/***********************************************************/
/* read a 16-bit word out of the EEPROM                    */
/***********************************************************/

int cs8900_e2prom_read(struct eth_device *dev,
			u8 addr, u16 *value)
{
	cs8900_e2prom_ready(dev);
	put_reg(dev, PP_EECMD, EEPROM_READ_CMD | addr);
	cs8900_e2prom_ready(dev);
	*value = get_reg(dev, PP_EEData);

	return 0;
}


/***********************************************************/
/* write a 16-bit word into the EEPROM                     */
/***********************************************************/

int cs8900_e2prom_write(struct eth_device *dev, u8 addr, u16 value)
{
	cs8900_e2prom_ready(dev);
	put_reg(dev, PP_EECMD, EEPROM_WRITE_EN);
	cs8900_e2prom_ready(dev);
	put_reg(dev, PP_EEData, value);
	put_reg(dev, PP_EECMD, EEPROM_WRITE_CMD | addr);
	cs8900_e2prom_ready(dev);
	put_reg(dev, PP_EECMD, EEPROM_WRITE_DIS);
	cs8900_e2prom_ready(dev);

	return 0;
}

int cs8900_initialize(u8 dev_num, int base_addr)
{
	struct eth_device *dev;
	struct cs8900_priv *priv;

	dev = malloc(sizeof(*dev));
	if (!dev) {
		return 0;
	}
	memset(dev, 0, sizeof(*dev));

	priv = malloc(sizeof(*priv));
	if (!priv) {
		free(dev);
		return 0;
	}
	memset(priv, 0, sizeof(*priv));
	priv->regs = (struct cs8900_regs *)base_addr;

	dev->iobase = base_addr;
	dev->priv = priv;
	dev->init = cs8900_init;
	dev->halt = cs8900_halt;
	dev->send = cs8900_send;
	dev->recv = cs8900_recv;

	/* Load MAC address from EEPROM */
	cs8900_get_enetaddr(dev);

	sprintf(dev->name, "%s-%hu", CS8900_DRIVERNAME, dev_num);

	eth_register(dev);
	return 0;
}
