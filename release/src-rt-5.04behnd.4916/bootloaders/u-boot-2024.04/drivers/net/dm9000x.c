// SPDX-License-Identifier: GPL-2.0+
/*
 *   dm9000.c: Version 1.2 12/15/2003
 *
 *	A Davicom DM9000 ISA NIC fast Ethernet driver for Linux.
 *	Copyright (C) 1997  Sten Wang
 *
 *   (C)Copyright 1997-1998 DAVICOM Semiconductor,Inc. All Rights Reserved.
 *
 * V0.11	06/20/2001	REG_0A bit3=1, default enable BP with DA match
 *	06/22/2001	Support DM9801 progrmming
 *			E3: R25 = ((R24 + NF) & 0x00ff) | 0xf000
 *			E4: R25 = ((R24 + NF) & 0x00ff) | 0xc200
 *		R17 = (R17 & 0xfff0) | NF + 3
 *			E5: R25 = ((R24 + NF - 3) & 0x00ff) | 0xc200
 *		R17 = (R17 & 0xfff0) | NF
 *
 * v1.00			modify by simon 2001.9.5
 *			change for kernel 2.4.x
 *
 * v1.1   11/09/2001	fix force mode bug
 *
 * v1.2   03/18/2003       Weilun Huang <weilun_huang@davicom.com.tw>:
 *			Fixed phy reset.
 *			Added tx/rx 32 bit mode.
 *			Cleaned up for kernel merge.
 *
 * --------------------------------------
 *
 *        12/15/2003       Initial port to u-boot by
 *			Sascha Hauer <saschahauer@web.de>
 *
 *        06/03/2008	Remy Bohmer <linux@bohmer.net>
 *			- Fixed the driver to work with DM9000A.
 *			  (check on ISR receive status bit before reading the
 *			  FIFO as described in DM9000 programming guide and
 *			  application notes)
 *			- Added autodetect of databus width.
 *			- Made debug code compile again.
 *			- Adapt eth_send such that it matches the DM9000*
 *			  application notes. Needed to make it work properly
 *			  for DM9000A.
 *			- Adapted reset procedure to match DM9000 application
 *			  notes (i.e. double reset)
 *			- some minor code cleanups
 *			These changes are tested with DM9000{A,EP,E} together
 *			with a 200MHz Atmel AT91SAM9261 core
 *
 * TODO: external MII is not functional, only internal at the moment.
 */

#include <common.h>
#include <command.h>
#include <dm.h>
#include <malloc.h>
#include <net.h>
#include <asm/io.h>
#include <linux/delay.h>

#include "dm9000x.h"

/* Structure/enum declaration ------------------------------- */
struct dm9000_priv {
	u32 runt_length_counter;	/* counter: RX length < 64byte */
	u32 long_length_counter;	/* counter: RX length > 1514byte */
	u32 reset_counter;	/* counter: RESET */
	u32 reset_tx_timeout;	/* RESET caused by TX Timeout */
	u32 reset_rx_status;	/* RESET caused by RX Statsus wrong */
	u16 tx_pkt_cnt;
	u16 queue_start_addr;
	u16 dbug_cnt;
	u8 phy_addr;
	u8 device_wait_reset;	/* device state */
	unsigned char srom[128];
	void (*outblk)(struct dm9000_priv *db, void *data_ptr, int count);
	void (*inblk)(struct dm9000_priv *db, void *data_ptr, int count);
	void (*rx_status)(struct dm9000_priv *db, u16 *rxstatus, u16 *rxlen);
	void __iomem *base_io;
	void __iomem *base_data;
};

/* DM9000 network board routine ---------------------------- */
#ifndef CONFIG_DM9000_BYTE_SWAPPED
#define dm9000_outb(d, r) writeb((d), (r))
#define dm9000_outw(d, r) writew((d), (r))
#define dm9000_outl(d, r) writel((d), (r))
#define dm9000_inb(r) readb(r)
#define dm9000_inw(r) readw(r)
#define dm9000_inl(r) readl(r)
#else
#define dm9000_outb(d, r) __raw_writeb(d, r)
#define dm9000_outw(d, r) __raw_writew(d, r)
#define dm9000_outl(d, r) __raw_writel(d, r)
#define dm9000_inb(r) __raw_readb(r)
#define dm9000_inw(r) __raw_readw(r)
#define dm9000_inl(r) __raw_readl(r)
#endif

#ifdef DEBUG
static void dm9000_dump_packet(const char *func, u8 *packet, int length)
{
	int i;

	printf("%s: length: %d\n", func, length);

	for (i = 0; i < length; i++) {
		if (i % 8 == 0)
			printf("\n%s: %02x: ", func, i);
		printf("%02x ", packet[i]);
	}

	printf("\n");
}
#else
static void dm9000_dump_packet(const char *func, u8 *packet, int length) {}
#endif

static void dm9000_outblk_8bit(struct dm9000_priv *db, void *data_ptr, int count)
{
	int i;

	for (i = 0; i < count; i++)
		dm9000_outb((((u8 *)data_ptr)[i] & 0xff), db->base_data);
}

static void dm9000_outblk_16bit(struct dm9000_priv *db, void *data_ptr, int count)
{
	int i;
	u32 tmplen = (count + 1) / 2;

	for (i = 0; i < tmplen; i++)
		dm9000_outw(((u16 *)data_ptr)[i], db->base_data);
}

static void dm9000_outblk_32bit(struct dm9000_priv *db, void *data_ptr, int count)
{
	int i;
	u32 tmplen = (count + 3) / 4;

	for (i = 0; i < tmplen; i++)
		dm9000_outl(((u32 *)data_ptr)[i], db->base_data);
}

static void dm9000_inblk_8bit(struct dm9000_priv *db, void *data_ptr, int count)
{
	int i;

	for (i = 0; i < count; i++)
		((u8 *)data_ptr)[i] = dm9000_inb(db->base_data);
}

static void dm9000_inblk_16bit(struct dm9000_priv *db, void *data_ptr, int count)
{
	int i;
	u32 tmplen = (count + 1) / 2;

	for (i = 0; i < tmplen; i++)
		((u16 *)data_ptr)[i] = dm9000_inw(db->base_data);
}

static void dm9000_inblk_32bit(struct dm9000_priv *db, void *data_ptr, int count)
{
	int i;
	u32 tmplen = (count + 3) / 4;

	for (i = 0; i < tmplen; i++)
		((u32 *)data_ptr)[i] = dm9000_inl(db->base_data);
}

static void dm9000_rx_status_32bit(struct dm9000_priv *db, u16 *rxstatus, u16 *rxlen)
{
	u32 tmpdata;

	dm9000_outb(DM9000_MRCMD, db->base_io);

	tmpdata = dm9000_inl(db->base_data);
	*rxstatus = __le16_to_cpu(tmpdata);
	*rxlen = __le16_to_cpu(tmpdata >> 16);
}

static void dm9000_rx_status_16bit(struct dm9000_priv *db, u16 *rxstatus, u16 *rxlen)
{
	dm9000_outb(DM9000_MRCMD, db->base_io);

	*rxstatus = __le16_to_cpu(dm9000_inw(db->base_data));
	*rxlen = __le16_to_cpu(dm9000_inw(db->base_data));
}

static void dm9000_rx_status_8bit(struct dm9000_priv *db, u16 *rxstatus, u16 *rxlen)
{
	dm9000_outb(DM9000_MRCMD, db->base_io);

	*rxstatus =
	    __le16_to_cpu(dm9000_inb(db->base_data) +
			  (dm9000_inb(db->base_data) << 8));
	*rxlen =
	    __le16_to_cpu(dm9000_inb(db->base_data) +
			  (dm9000_inb(db->base_data) << 8));
}

/*
 *  Read a byte from I/O port
 */
static u8 dm9000_ior(struct dm9000_priv *db, int reg)
{
	dm9000_outb(reg, db->base_io);
	return dm9000_inb(db->base_data);
}

/*
 *  Write a byte to I/O port
 */
static void dm9000_iow(struct dm9000_priv *db, int reg, u8 value)
{
	dm9000_outb(reg, db->base_io);
	dm9000_outb(value, db->base_data);
}

/*
 *  Read a word from phyxcer
 */
static u16 dm9000_phy_read(struct dm9000_priv *db, int reg)
{
	u16 val;

	/* Fill the phyxcer register into REG_0C */
	dm9000_iow(db, DM9000_EPAR, DM9000_PHY | reg);
	dm9000_iow(db, DM9000_EPCR, 0xc);	/* Issue phyxcer read command */
	udelay(100);			/* Wait read complete */
	dm9000_iow(db, DM9000_EPCR, 0x0);	/* Clear phyxcer read command */
	val = (dm9000_ior(db, DM9000_EPDRH) << 8) |
	      dm9000_ior(db, DM9000_EPDRL);

	/* The read data keeps on REG_0D & REG_0E */
	debug("%s(0x%x): 0x%x\n", __func__, reg, val);
	return val;
}

/*
 *  Write a word to phyxcer
 */
static void dm9000_phy_write(struct dm9000_priv *db, int reg, u16 value)
{
	/* Fill the phyxcer register into REG_0C */
	dm9000_iow(db, DM9000_EPAR, DM9000_PHY | reg);

	/* Fill the written data into REG_0D & REG_0E */
	dm9000_iow(db, DM9000_EPDRL, (value & 0xff));
	dm9000_iow(db, DM9000_EPDRH, ((value >> 8) & 0xff));
	dm9000_iow(db, DM9000_EPCR, 0xa);	/* Issue phyxcer write command */
	udelay(500);			/* Wait write complete */
	dm9000_iow(db, DM9000_EPCR, 0x0);	/* Clear phyxcer write command */
	debug("%s(reg:0x%x, value:0x%x)\n", __func__, reg, value);
}

/*
 * Search DM9000 board, allocate space and register it
 */
static int dm9000_probe(struct dm9000_priv *db)
{
	u32 id_val;

	id_val = dm9000_ior(db, DM9000_VIDL);
	id_val |= dm9000_ior(db, DM9000_VIDH) << 8;
	id_val |= dm9000_ior(db, DM9000_PIDL) << 16;
	id_val |= dm9000_ior(db, DM9000_PIDH) << 24;
	if (id_val != DM9000_ID) {
		printf("dm9000 not found at 0x%p id: 0x%08x\n",
		       db->base_io, id_val);
		return -1;
	}

	printf("dm9000 i/o: 0x%p, id: 0x%x\n", db->base_io, id_val);
	return 0;
}

/* General Purpose dm9000 reset routine */
static void dm9000_reset(struct dm9000_priv *db)
{
	debug("resetting DM9000\n");

	/*
	 * Reset DM9000,
	 * see DM9000 Application Notes V1.22 Jun 11, 2004 page 29
	 */

	/* DEBUG: Make all GPIO0 outputs, all others inputs */
	dm9000_iow(db, DM9000_GPCR, GPCR_GPIO0_OUT);
	/* Step 1: Power internal PHY by writing 0 to GPIO0 pin */
	dm9000_iow(db, DM9000_GPR, 0);
	/* Step 2: Software reset */
	dm9000_iow(db, DM9000_NCR, (NCR_LBK_INT_MAC | NCR_RST));

	do {
		debug("resetting the DM9000, 1st reset\n");
		udelay(25); /* Wait at least 20 us */
	} while (dm9000_ior(db, DM9000_NCR) & 1);

	dm9000_iow(db, DM9000_NCR, 0);
	dm9000_iow(db, DM9000_NCR, (NCR_LBK_INT_MAC | NCR_RST)); /* Issue a second reset */

	do {
		debug("resetting the DM9000, 2nd reset\n");
		udelay(25); /* Wait at least 20 us */
	} while (dm9000_ior(db, DM9000_NCR) & 1);

	/* Check whether the ethernet controller is present */
	if ((dm9000_ior(db, DM9000_PIDL) != 0x0) ||
	    (dm9000_ior(db, DM9000_PIDH) != 0x90))
		printf("ERROR: resetting DM9000 -> not responding\n");
}

/* Initialize dm9000 board */
static int dm9000_init_common(struct dm9000_priv *db, u8 enetaddr[6])
{
	int i, oft, lnk;
	u8 io_mode;

	/* RESET device */
	dm9000_reset(db);

	if (dm9000_probe(db) < 0)
		return -1;

	/* Auto-detect 8/16/32 bit mode, ISR Bit 6+7 indicate bus width */
	io_mode = dm9000_ior(db, DM9000_ISR) >> 6;

	switch (io_mode) {
	case 0x0:  /* 16-bit mode */
		printf("DM9000: running in 16 bit mode\n");
		db->outblk    = dm9000_outblk_16bit;
		db->inblk     = dm9000_inblk_16bit;
		db->rx_status = dm9000_rx_status_16bit;
		break;
	case 0x01:  /* 32-bit mode */
		printf("DM9000: running in 32 bit mode\n");
		db->outblk    = dm9000_outblk_32bit;
		db->inblk     = dm9000_inblk_32bit;
		db->rx_status = dm9000_rx_status_32bit;
		break;
	case 0x02: /* 8 bit mode */
		printf("DM9000: running in 8 bit mode\n");
		db->outblk    = dm9000_outblk_8bit;
		db->inblk     = dm9000_inblk_8bit;
		db->rx_status = dm9000_rx_status_8bit;
		break;
	default:
		/* Assume 8 bit mode, will probably not work anyway */
		printf("DM9000: Undefined IO-mode:0x%x\n", io_mode);
		db->outblk    = dm9000_outblk_8bit;
		db->inblk     = dm9000_inblk_8bit;
		db->rx_status = dm9000_rx_status_8bit;
		break;
	}

	/* Program operating register, only internal phy supported */
	dm9000_iow(db, DM9000_NCR, 0x0);
	/* TX Polling clear */
	dm9000_iow(db, DM9000_TCR, 0);
	/* Less 3Kb, 200us */
	dm9000_iow(db, DM9000_BPTR, BPTR_BPHW(3) | BPTR_JPT_600US);
	/* Flow Control : High/Low Water */
	dm9000_iow(db, DM9000_FCTR, FCTR_HWOT(3) | FCTR_LWOT(8));
	/* SH FIXME: This looks strange! Flow Control */
	dm9000_iow(db, DM9000_FCR, 0x0);
	/* Special Mode */
	dm9000_iow(db, DM9000_SMCR, 0);
	/* clear TX status */
	dm9000_iow(db, DM9000_NSR, NSR_WAKEST | NSR_TX2END | NSR_TX1END);
	/* Clear interrupt status */
	dm9000_iow(db, DM9000_ISR, ISR_ROOS | ISR_ROS | ISR_PTS | ISR_PRS);

	printf("MAC: %pM\n", enetaddr);
	if (!is_valid_ethaddr(enetaddr))
		printf("WARNING: Bad MAC address (uninitialized EEPROM?)\n");

	/* fill device MAC address registers */
	for (i = 0, oft = DM9000_PAR; i < 6; i++, oft++)
		dm9000_iow(db, oft, enetaddr[i]);
	for (i = 0, oft = 0x16; i < 8; i++, oft++)
		dm9000_iow(db, oft, 0xff);

	/* read back mac, just to be sure */
	for (i = 0, oft = 0x10; i < 6; i++, oft++)
		debug("%02x:", dm9000_ior(db, oft));
	debug("\n");

	/* Activate DM9000 */
	/* RX enable */
	dm9000_iow(db, DM9000_RCR, RCR_DIS_LONG | RCR_DIS_CRC | RCR_RXEN);
	/* Enable TX/RX interrupt mask */
	dm9000_iow(db, DM9000_IMR, IMR_PAR);

	i = 0;
	while (!(dm9000_phy_read(db, 1) & 0x20)) {	/* autonegation complete bit */
		udelay(1000);
		i++;
		if (i == 10000) {
			printf("could not establish link\n");
			return 0;
		}
	}

	/* see what we've got */
	lnk = dm9000_phy_read(db, 17) >> 12;
	printf("operating at ");
	switch (lnk) {
	case 1:
		printf("10M half duplex ");
		break;
	case 2:
		printf("10M full duplex ");
		break;
	case 4:
		printf("100M half duplex ");
		break;
	case 8:
		printf("100M full duplex ");
		break;
	default:
		printf("unknown: %d ", lnk);
		break;
	}
	printf("mode\n");
	return 0;
}

/*
 * Hardware start transmission.
 * Send a packet to media from the upper layer.
 */
static int dm9000_send_common(struct dm9000_priv *db, void *packet, int length)
{
	int tmo;

	dm9000_dump_packet(__func__, packet, length);

	dm9000_iow(db, DM9000_ISR, IMR_PTM); /* Clear Tx bit in ISR */

	/* Move data to DM9000 TX RAM */
	dm9000_outb(DM9000_MWCMD, db->base_io); /* Prepare for TX-data */

	/* push the data to the TX-fifo */
	db->outblk(db, packet, length);

	/* Set TX length to DM9000 */
	dm9000_iow(db, DM9000_TXPLL, length & 0xff);
	dm9000_iow(db, DM9000_TXPLH, (length >> 8) & 0xff);

	/* Issue TX polling command */
	dm9000_iow(db, DM9000_TCR, TCR_TXREQ); /* Cleared after TX complete */

	/* wait for end of transmission */
	tmo = get_timer(0) + 5 * CONFIG_SYS_HZ;
	while (!(dm9000_ior(db, DM9000_NSR) & (NSR_TX1END | NSR_TX2END)) ||
	       !(dm9000_ior(db, DM9000_ISR) & IMR_PTM)) {
		if (get_timer(0) >= tmo) {
			printf("transmission timeout\n");
			break;
		}
	}
	dm9000_iow(db, DM9000_ISR, IMR_PTM); /* Clear Tx bit in ISR */

	debug("transmit done\n\n");
	return 0;
}

/*
 * Stop the interface.
 * The interface is stopped when it is brought.
 */
static void dm9000_halt_common(struct dm9000_priv *db)
{
	/* RESET device */
	dm9000_phy_write(db, 0, 0x8000);	/* PHY RESET */
	dm9000_iow(db, DM9000_GPR, 0x01);	/* Power-Down PHY */
	dm9000_iow(db, DM9000_IMR, 0x80);	/* Disable all interrupt */
	dm9000_iow(db, DM9000_RCR, 0x00);	/* Disable RX */
}

/*
 * Received a packet and pass to upper layer
 */
static int dm9000_recv_common(struct dm9000_priv *db, uchar *rdptr)
{
	u8 rxbyte;
	u16 rxstatus, rxlen = 0;

	/*
	 * Check packet ready or not, we must check
	 * the ISR status first for DM9000A
	 */
	if (!(dm9000_ior(db, DM9000_ISR) & 0x01)) /* Rx-ISR bit must be set. */
		return 0;

	dm9000_iow(db, DM9000_ISR, 0x01); /* clear PR status latched in bit 0 */

	/* There is _at least_ 1 package in the fifo, read them all */
	dm9000_ior(db, DM9000_MRCMDX);	/* Dummy read */

	/*
	 * Get most updated data,
	 * only look at bits 0:1, See application notes DM9000
	 */
	rxbyte = dm9000_inb(db->base_data) & 0x03;

	/* Status check: this byte must be 0 or 1 */
	if (rxbyte > DM9000_PKT_RDY) {
		dm9000_iow(db, DM9000_RCR, 0x00);	/* Stop Device */
		dm9000_iow(db, DM9000_ISR, 0x80);	/* Stop INT request */
		printf("DM9000 error: status check fail: 0x%x\n",
		       rxbyte);
		return -EINVAL;
	}

	if (rxbyte != DM9000_PKT_RDY)
		return 0; /* No packet received, ignore */

	debug("receiving packet\n");

	/* A packet ready now  & Get status/length */
	db->rx_status(db, &rxstatus, &rxlen);

	debug("rx status: 0x%04x rx len: %d\n", rxstatus, rxlen);

	/* Move data from DM9000 */
	/* Read received packet from RX SRAM */
	db->inblk(db, rdptr, rxlen);

	if (rxstatus & 0xbf00 || rxlen < 0x40 || rxlen > DM9000_PKT_MAX) {
		if (rxstatus & 0x100)
			printf("rx fifo error\n");
		if (rxstatus & 0x200)
			printf("rx crc error\n");
		if (rxstatus & 0x8000)
			printf("rx length error\n");
		if (rxlen > DM9000_PKT_MAX) {
			printf("rx length too big\n");
			dm9000_reset(db);
		}
		return -EINVAL;
	}

	return rxlen;
}

/*
 * Read a word data from SROM
 */
#if !defined(CONFIG_DM9000_NO_SROM)
static void dm9000_read_srom_word(struct dm9000_priv *db, int offset, u8 *to)
{
	dm9000_iow(db, DM9000_EPAR, offset);
	dm9000_iow(db, DM9000_EPCR, 0x4);
	mdelay(8);
	dm9000_iow(db, DM9000_EPCR, 0x0);
	to[0] = dm9000_ior(db, DM9000_EPDRL);
	to[1] = dm9000_ior(db, DM9000_EPDRH);
}

static void dm9000_get_enetaddr(struct dm9000_priv *db, u8 *enetaddr)
{
	int i;

	for (i = 0; i < 3; i++)
		dm9000_read_srom_word(db, i, enetaddr + (2 * i));
}
#else
static void dm9000_get_enetaddr(struct dm9000_priv *db, u8 *enetaddr) {}
#endif

static int dm9000_start(struct udevice *dev)
{
	struct dm9000_priv *db = dev_get_priv(dev);
	struct eth_pdata *pdata = dev_get_plat(dev);

	return dm9000_init_common(db, pdata->enetaddr);
}

static void dm9000_stop(struct udevice *dev)
{
	struct dm9000_priv *db = dev_get_priv(dev);

	dm9000_halt_common(db);
}

static int dm9000_send(struct udevice *dev, void *packet, int length)
{
	struct dm9000_priv *db = dev_get_priv(dev);
	int ret;

	ret = dm9000_send_common(db, packet, length);

	return ret ? 0 : -ETIMEDOUT;
}

static int dm9000_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct dm9000_priv *db = dev_get_priv(dev);
	uchar *data = net_rx_packets[0];
	int ret;

	ret = dm9000_recv_common(db, data);
	if (ret > 0)
		*packetp = (void *)data;

	return ret >= 0 ? ret : -EAGAIN;
}

static int dm9000_write_hwaddr(struct udevice *dev)
{
	struct dm9000_priv *db = dev_get_priv(dev);
	struct eth_pdata *pdata = dev_get_plat(dev);
	int i, oft;

	/* fill device MAC address registers */
	for (i = 0, oft = DM9000_PAR; i < 6; i++, oft++)
		dm9000_iow(db, oft, pdata->enetaddr[i]);

	for (i = 0, oft = 0x16; i < 8; i++, oft++)
		dm9000_iow(db, oft, 0xff);

	/* read back mac, just to be sure */
	for (i = 0, oft = 0x10; i < 6; i++, oft++)
		debug("%02x:", dm9000_ior(db, oft));

	debug("\n");

	return 0;
}

static int dm9000_read_rom_hwaddr(struct udevice *dev)
{
	struct dm9000_priv *db = dev_get_priv(dev);
	struct eth_pdata *pdata = dev_get_plat(dev);

	dm9000_get_enetaddr(db, pdata->enetaddr);

	return !is_valid_ethaddr(pdata->enetaddr);
}

static int dm9000_bind(struct udevice *dev)
{
	return device_set_name(dev, dev->name);
}

static int dm9000_of_to_plat(struct udevice *dev)
{
	struct dm9000_priv *db = dev_get_priv(dev);
	struct eth_pdata *pdata = dev_get_plat(dev);

	pdata->iobase = dev_read_addr_index(dev, 0);
	db->base_io = (void __iomem *)pdata->iobase;
	db->base_data = dev_read_addr_index_ptr(dev, 1);

	return 0;
}

static const struct eth_ops dm9000_ops = {
	.start		= dm9000_start,
	.stop		= dm9000_stop,
	.send		= dm9000_send,
	.recv		= dm9000_recv,
	.write_hwaddr	= dm9000_write_hwaddr,
	.read_rom_hwaddr = dm9000_read_rom_hwaddr,
};

static const struct udevice_id dm9000_ids[] = {
	{ .compatible = "davicom,dm9000" },
	{ }
};

U_BOOT_DRIVER(dm9000) = {
	.name		= "eth_dm9000",
	.id		= UCLASS_ETH,
	.of_match	= dm9000_ids,
	.bind		= dm9000_bind,
	.of_to_plat = dm9000_of_to_plat,
	.ops		= &dm9000_ops,
	.priv_auto	= sizeof(struct dm9000_priv),
	.plat_auto	= sizeof(struct eth_pdata),
	.flags		= DM_FLAG_ALLOC_PRIV_DMA,
};
