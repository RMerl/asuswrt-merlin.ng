// SPDX-License-Identifier: GPL-2.0+
/*
  dm9000.c: Version 1.2 12/15/2003

	A Davicom DM9000 ISA NIC fast Ethernet driver for Linux.
	Copyright (C) 1997  Sten Wang

  (C)Copyright 1997-1998 DAVICOM Semiconductor,Inc. All Rights Reserved.

V0.11	06/20/2001	REG_0A bit3=1, default enable BP with DA match
	06/22/2001	Support DM9801 progrmming
			E3: R25 = ((R24 + NF) & 0x00ff) | 0xf000
			E4: R25 = ((R24 + NF) & 0x00ff) | 0xc200
		R17 = (R17 & 0xfff0) | NF + 3
			E5: R25 = ((R24 + NF - 3) & 0x00ff) | 0xc200
		R17 = (R17 & 0xfff0) | NF

v1.00			modify by simon 2001.9.5
			change for kernel 2.4.x

v1.1   11/09/2001	fix force mode bug

v1.2   03/18/2003       Weilun Huang <weilun_huang@davicom.com.tw>:
			Fixed phy reset.
			Added tx/rx 32 bit mode.
			Cleaned up for kernel merge.

--------------------------------------

       12/15/2003       Initial port to u-boot by
       			Sascha Hauer <saschahauer@web.de>

       06/03/2008	Remy Bohmer <linux@bohmer.net>
			- Fixed the driver to work with DM9000A.
			  (check on ISR receive status bit before reading the
			  FIFO as described in DM9000 programming guide and
			  application notes)
			- Added autodetect of databus width.
			- Made debug code compile again.
			- Adapt eth_send such that it matches the DM9000*
			  application notes. Needed to make it work properly
			  for DM9000A.
			- Adapted reset procedure to match DM9000 application
			  notes (i.e. double reset)
			- some minor code cleanups
			These changes are tested with DM9000{A,EP,E} together
			with a 200MHz Atmel AT91SAM9261 core

TODO: external MII is not functional, only internal at the moment.
*/

#include <common.h>
#include <command.h>
#include <net.h>
#include <asm/io.h>
#include <dm9000.h>

#include "dm9000x.h"

/* Board/System/Debug information/definition ---------------- */

/* #define CONFIG_DM9000_DEBUG */

#ifdef CONFIG_DM9000_DEBUG
#define DM9000_DBG(fmt,args...) printf(fmt, ##args)
#define DM9000_DMP_PACKET(func,packet,length)  \
	do { \
		int i; 							\
		printf("%s: length: %d\n", func, length);		\
		for (i = 0; i < length; i++) {				\
			if (i % 8 == 0)					\
				printf("\n%s: %02x: ", func, i);	\
			printf("%02x ", ((unsigned char *) packet)[i]);	\
		} printf("\n");						\
	} while(0)
#else
#define DM9000_DBG(fmt,args...)
#define DM9000_DMP_PACKET(func,packet,length)
#endif

/* Structure/enum declaration ------------------------------- */
typedef struct board_info {
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
	void (*outblk)(volatile void *data_ptr, int count);
	void (*inblk)(void *data_ptr, int count);
	void (*rx_status)(u16 *RxStatus, u16 *RxLen);
	struct eth_device netdev;
} board_info_t;
static board_info_t dm9000_info;


/* function declaration ------------------------------------- */
static int dm9000_probe(void);
static u16 dm9000_phy_read(int);
static void dm9000_phy_write(int, u16);
static u8 DM9000_ior(int);
static void DM9000_iow(int reg, u8 value);

/* DM9000 network board routine ---------------------------- */
#ifndef CONFIG_DM9000_BYTE_SWAPPED
#define DM9000_outb(d,r) writeb(d, (volatile u8 *)(r))
#define DM9000_outw(d,r) writew(d, (volatile u16 *)(r))
#define DM9000_outl(d,r) writel(d, (volatile u32 *)(r))
#define DM9000_inb(r) readb((volatile u8 *)(r))
#define DM9000_inw(r) readw((volatile u16 *)(r))
#define DM9000_inl(r) readl((volatile u32 *)(r))
#else
#define DM9000_outb(d, r) __raw_writeb(d, r)
#define DM9000_outw(d, r) __raw_writew(d, r)
#define DM9000_outl(d, r) __raw_writel(d, r)
#define DM9000_inb(r) __raw_readb(r)
#define DM9000_inw(r) __raw_readw(r)
#define DM9000_inl(r) __raw_readl(r)
#endif

#ifdef CONFIG_DM9000_DEBUG
static void
dump_regs(void)
{
	DM9000_DBG("\n");
	DM9000_DBG("NCR   (0x00): %02x\n", DM9000_ior(0));
	DM9000_DBG("NSR   (0x01): %02x\n", DM9000_ior(1));
	DM9000_DBG("TCR   (0x02): %02x\n", DM9000_ior(2));
	DM9000_DBG("TSRI  (0x03): %02x\n", DM9000_ior(3));
	DM9000_DBG("TSRII (0x04): %02x\n", DM9000_ior(4));
	DM9000_DBG("RCR   (0x05): %02x\n", DM9000_ior(5));
	DM9000_DBG("RSR   (0x06): %02x\n", DM9000_ior(6));
	DM9000_DBG("ISR   (0xFE): %02x\n", DM9000_ior(DM9000_ISR));
	DM9000_DBG("\n");
}
#endif

static void dm9000_outblk_8bit(volatile void *data_ptr, int count)
{
	int i;
	for (i = 0; i < count; i++)
		DM9000_outb((((u8 *) data_ptr)[i] & 0xff), DM9000_DATA);
}

static void dm9000_outblk_16bit(volatile void *data_ptr, int count)
{
	int i;
	u32 tmplen = (count + 1) / 2;

	for (i = 0; i < tmplen; i++)
		DM9000_outw(((u16 *) data_ptr)[i], DM9000_DATA);
}
static void dm9000_outblk_32bit(volatile void *data_ptr, int count)
{
	int i;
	u32 tmplen = (count + 3) / 4;

	for (i = 0; i < tmplen; i++)
		DM9000_outl(((u32 *) data_ptr)[i], DM9000_DATA);
}

static void dm9000_inblk_8bit(void *data_ptr, int count)
{
	int i;
	for (i = 0; i < count; i++)
		((u8 *) data_ptr)[i] = DM9000_inb(DM9000_DATA);
}

static void dm9000_inblk_16bit(void *data_ptr, int count)
{
	int i;
	u32 tmplen = (count + 1) / 2;

	for (i = 0; i < tmplen; i++)
		((u16 *) data_ptr)[i] = DM9000_inw(DM9000_DATA);
}
static void dm9000_inblk_32bit(void *data_ptr, int count)
{
	int i;
	u32 tmplen = (count + 3) / 4;

	for (i = 0; i < tmplen; i++)
		((u32 *) data_ptr)[i] = DM9000_inl(DM9000_DATA);
}

static void dm9000_rx_status_32bit(u16 *RxStatus, u16 *RxLen)
{
	u32 tmpdata;

	DM9000_outb(DM9000_MRCMD, DM9000_IO);

	tmpdata = DM9000_inl(DM9000_DATA);
	*RxStatus = __le16_to_cpu(tmpdata);
	*RxLen = __le16_to_cpu(tmpdata >> 16);
}

static void dm9000_rx_status_16bit(u16 *RxStatus, u16 *RxLen)
{
	DM9000_outb(DM9000_MRCMD, DM9000_IO);

	*RxStatus = __le16_to_cpu(DM9000_inw(DM9000_DATA));
	*RxLen = __le16_to_cpu(DM9000_inw(DM9000_DATA));
}

static void dm9000_rx_status_8bit(u16 *RxStatus, u16 *RxLen)
{
	DM9000_outb(DM9000_MRCMD, DM9000_IO);

	*RxStatus =
	    __le16_to_cpu(DM9000_inb(DM9000_DATA) +
			  (DM9000_inb(DM9000_DATA) << 8));
	*RxLen =
	    __le16_to_cpu(DM9000_inb(DM9000_DATA) +
			  (DM9000_inb(DM9000_DATA) << 8));
}

/*
  Search DM9000 board, allocate space and register it
*/
int
dm9000_probe(void)
{
	u32 id_val;
	id_val = DM9000_ior(DM9000_VIDL);
	id_val |= DM9000_ior(DM9000_VIDH) << 8;
	id_val |= DM9000_ior(DM9000_PIDL) << 16;
	id_val |= DM9000_ior(DM9000_PIDH) << 24;
	if (id_val == DM9000_ID) {
		printf("dm9000 i/o: 0x%x, id: 0x%x \n", CONFIG_DM9000_BASE,
		       id_val);
		return 0;
	} else {
		printf("dm9000 not found at 0x%08x id: 0x%08x\n",
		       CONFIG_DM9000_BASE, id_val);
		return -1;
	}
}

/* General Purpose dm9000 reset routine */
static void
dm9000_reset(void)
{
	DM9000_DBG("resetting DM9000\n");

	/* Reset DM9000,
	   see DM9000 Application Notes V1.22 Jun 11, 2004 page 29 */

	/* DEBUG: Make all GPIO0 outputs, all others inputs */
	DM9000_iow(DM9000_GPCR, GPCR_GPIO0_OUT);
	/* Step 1: Power internal PHY by writing 0 to GPIO0 pin */
	DM9000_iow(DM9000_GPR, 0);
	/* Step 2: Software reset */
	DM9000_iow(DM9000_NCR, (NCR_LBK_INT_MAC | NCR_RST));

	do {
		DM9000_DBG("resetting the DM9000, 1st reset\n");
		udelay(25); /* Wait at least 20 us */
	} while (DM9000_ior(DM9000_NCR) & 1);

	DM9000_iow(DM9000_NCR, 0);
	DM9000_iow(DM9000_NCR, (NCR_LBK_INT_MAC | NCR_RST)); /* Issue a second reset */

	do {
		DM9000_DBG("resetting the DM9000, 2nd reset\n");
		udelay(25); /* Wait at least 20 us */
	} while (DM9000_ior(DM9000_NCR) & 1);

	/* Check whether the ethernet controller is present */
	if ((DM9000_ior(DM9000_PIDL) != 0x0) ||
	    (DM9000_ior(DM9000_PIDH) != 0x90))
		printf("ERROR: resetting DM9000 -> not responding\n");
}

/* Initialize dm9000 board
*/
static int dm9000_init(struct eth_device *dev, bd_t *bd)
{
	int i, oft, lnk;
	u8 io_mode;
	struct board_info *db = &dm9000_info;

	DM9000_DBG("%s\n", __func__);

	/* RESET device */
	dm9000_reset();

	if (dm9000_probe() < 0)
		return -1;

	/* Auto-detect 8/16/32 bit mode, ISR Bit 6+7 indicate bus width */
	io_mode = DM9000_ior(DM9000_ISR) >> 6;

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
	DM9000_iow(DM9000_NCR, 0x0);
	/* TX Polling clear */
	DM9000_iow(DM9000_TCR, 0);
	/* Less 3Kb, 200us */
	DM9000_iow(DM9000_BPTR, BPTR_BPHW(3) | BPTR_JPT_600US);
	/* Flow Control : High/Low Water */
	DM9000_iow(DM9000_FCTR, FCTR_HWOT(3) | FCTR_LWOT(8));
	/* SH FIXME: This looks strange! Flow Control */
	DM9000_iow(DM9000_FCR, 0x0);
	/* Special Mode */
	DM9000_iow(DM9000_SMCR, 0);
	/* clear TX status */
	DM9000_iow(DM9000_NSR, NSR_WAKEST | NSR_TX2END | NSR_TX1END);
	/* Clear interrupt status */
	DM9000_iow(DM9000_ISR, ISR_ROOS | ISR_ROS | ISR_PTS | ISR_PRS);

	printf("MAC: %pM\n", dev->enetaddr);
	if (!is_valid_ethaddr(dev->enetaddr)) {
		printf("WARNING: Bad MAC address (uninitialized EEPROM?)\n");
	}

	/* fill device MAC address registers */
	for (i = 0, oft = DM9000_PAR; i < 6; i++, oft++)
		DM9000_iow(oft, dev->enetaddr[i]);
	for (i = 0, oft = 0x16; i < 8; i++, oft++)
		DM9000_iow(oft, 0xff);

	/* read back mac, just to be sure */
	for (i = 0, oft = 0x10; i < 6; i++, oft++)
		DM9000_DBG("%02x:", DM9000_ior(oft));
	DM9000_DBG("\n");

	/* Activate DM9000 */
	/* RX enable */
	DM9000_iow(DM9000_RCR, RCR_DIS_LONG | RCR_DIS_CRC | RCR_RXEN);
	/* Enable TX/RX interrupt mask */
	DM9000_iow(DM9000_IMR, IMR_PAR);

	i = 0;
	while (!(dm9000_phy_read(1) & 0x20)) {	/* autonegation complete bit */
		udelay(1000);
		i++;
		if (i == 10000) {
			printf("could not establish link\n");
			return 0;
		}
	}

	/* see what we've got */
	lnk = dm9000_phy_read(17) >> 12;
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
  Hardware start transmission.
  Send a packet to media from the upper layer.
*/
static int dm9000_send(struct eth_device *netdev, void *packet, int length)
{
	int tmo;
	struct board_info *db = &dm9000_info;

	DM9000_DMP_PACKET(__func__ , packet, length);

	DM9000_iow(DM9000_ISR, IMR_PTM); /* Clear Tx bit in ISR */

	/* Move data to DM9000 TX RAM */
	DM9000_outb(DM9000_MWCMD, DM9000_IO); /* Prepare for TX-data */

	/* push the data to the TX-fifo */
	(db->outblk)(packet, length);

	/* Set TX length to DM9000 */
	DM9000_iow(DM9000_TXPLL, length & 0xff);
	DM9000_iow(DM9000_TXPLH, (length >> 8) & 0xff);

	/* Issue TX polling command */
	DM9000_iow(DM9000_TCR, TCR_TXREQ); /* Cleared after TX complete */

	/* wait for end of transmission */
	tmo = get_timer(0) + 5 * CONFIG_SYS_HZ;
	while ( !(DM9000_ior(DM9000_NSR) & (NSR_TX1END | NSR_TX2END)) ||
		!(DM9000_ior(DM9000_ISR) & IMR_PTM) ) {
		if (get_timer(0) >= tmo) {
			printf("transmission timeout\n");
			break;
		}
	}
	DM9000_iow(DM9000_ISR, IMR_PTM); /* Clear Tx bit in ISR */

	DM9000_DBG("transmit done\n\n");
	return 0;
}

/*
  Stop the interface.
  The interface is stopped when it is brought.
*/
static void dm9000_halt(struct eth_device *netdev)
{
	DM9000_DBG("%s\n", __func__);

	/* RESET devie */
	dm9000_phy_write(0, 0x8000);	/* PHY RESET */
	DM9000_iow(DM9000_GPR, 0x01);	/* Power-Down PHY */
	DM9000_iow(DM9000_IMR, 0x80);	/* Disable all interrupt */
	DM9000_iow(DM9000_RCR, 0x00);	/* Disable RX */
}

/*
  Received a packet and pass to upper layer
*/
static int dm9000_rx(struct eth_device *netdev)
{
	u8 rxbyte;
	u8 *rdptr = (u8 *)net_rx_packets[0];
	u16 RxStatus, RxLen = 0;
	struct board_info *db = &dm9000_info;

	/* Check packet ready or not, we must check
	   the ISR status first for DM9000A */
	if (!(DM9000_ior(DM9000_ISR) & 0x01)) /* Rx-ISR bit must be set. */
		return 0;

	DM9000_iow(DM9000_ISR, 0x01); /* clear PR status latched in bit 0 */

	/* There is _at least_ 1 package in the fifo, read them all */
	for (;;) {
		DM9000_ior(DM9000_MRCMDX);	/* Dummy read */

		/* Get most updated data,
		   only look at bits 0:1, See application notes DM9000 */
		rxbyte = DM9000_inb(DM9000_DATA) & 0x03;

		/* Status check: this byte must be 0 or 1 */
		if (rxbyte > DM9000_PKT_RDY) {
			DM9000_iow(DM9000_RCR, 0x00);	/* Stop Device */
			DM9000_iow(DM9000_ISR, 0x80);	/* Stop INT request */
			printf("DM9000 error: status check fail: 0x%x\n",
				rxbyte);
			return 0;
		}

		if (rxbyte != DM9000_PKT_RDY)
			return 0; /* No packet received, ignore */

		DM9000_DBG("receiving packet\n");

		/* A packet ready now  & Get status/length */
		(db->rx_status)(&RxStatus, &RxLen);

		DM9000_DBG("rx status: 0x%04x rx len: %d\n", RxStatus, RxLen);

		/* Move data from DM9000 */
		/* Read received packet from RX SRAM */
		(db->inblk)(rdptr, RxLen);

		if ((RxStatus & 0xbf00) || (RxLen < 0x40)
			|| (RxLen > DM9000_PKT_MAX)) {
			if (RxStatus & 0x100) {
				printf("rx fifo error\n");
			}
			if (RxStatus & 0x200) {
				printf("rx crc error\n");
			}
			if (RxStatus & 0x8000) {
				printf("rx length error\n");
			}
			if (RxLen > DM9000_PKT_MAX) {
				printf("rx length too big\n");
				dm9000_reset();
			}
		} else {
			DM9000_DMP_PACKET(__func__ , rdptr, RxLen);

			DM9000_DBG("passing packet to upper layer\n");
			net_process_received_packet(net_rx_packets[0], RxLen);
		}
	}
	return 0;
}

/*
  Read a word data from SROM
*/
#if !defined(CONFIG_DM9000_NO_SROM)
void dm9000_read_srom_word(int offset, u8 *to)
{
	DM9000_iow(DM9000_EPAR, offset);
	DM9000_iow(DM9000_EPCR, 0x4);
	udelay(8000);
	DM9000_iow(DM9000_EPCR, 0x0);
	to[0] = DM9000_ior(DM9000_EPDRL);
	to[1] = DM9000_ior(DM9000_EPDRH);
}

void dm9000_write_srom_word(int offset, u16 val)
{
	DM9000_iow(DM9000_EPAR, offset);
	DM9000_iow(DM9000_EPDRH, ((val >> 8) & 0xff));
	DM9000_iow(DM9000_EPDRL, (val & 0xff));
	DM9000_iow(DM9000_EPCR, 0x12);
	udelay(8000);
	DM9000_iow(DM9000_EPCR, 0);
}
#endif

static void dm9000_get_enetaddr(struct eth_device *dev)
{
#if !defined(CONFIG_DM9000_NO_SROM)
	int i;
	for (i = 0; i < 3; i++)
		dm9000_read_srom_word(i, dev->enetaddr + (2 * i));
#endif
}

/*
   Read a byte from I/O port
*/
static u8
DM9000_ior(int reg)
{
	DM9000_outb(reg, DM9000_IO);
	return DM9000_inb(DM9000_DATA);
}

/*
   Write a byte to I/O port
*/
static void
DM9000_iow(int reg, u8 value)
{
	DM9000_outb(reg, DM9000_IO);
	DM9000_outb(value, DM9000_DATA);
}

/*
   Read a word from phyxcer
*/
static u16
dm9000_phy_read(int reg)
{
	u16 val;

	/* Fill the phyxcer register into REG_0C */
	DM9000_iow(DM9000_EPAR, DM9000_PHY | reg);
	DM9000_iow(DM9000_EPCR, 0xc);	/* Issue phyxcer read command */
	udelay(100);			/* Wait read complete */
	DM9000_iow(DM9000_EPCR, 0x0);	/* Clear phyxcer read command */
	val = (DM9000_ior(DM9000_EPDRH) << 8) | DM9000_ior(DM9000_EPDRL);

	/* The read data keeps on REG_0D & REG_0E */
	DM9000_DBG("dm9000_phy_read(0x%x): 0x%x\n", reg, val);
	return val;
}

/*
   Write a word to phyxcer
*/
static void
dm9000_phy_write(int reg, u16 value)
{

	/* Fill the phyxcer register into REG_0C */
	DM9000_iow(DM9000_EPAR, DM9000_PHY | reg);

	/* Fill the written data into REG_0D & REG_0E */
	DM9000_iow(DM9000_EPDRL, (value & 0xff));
	DM9000_iow(DM9000_EPDRH, ((value >> 8) & 0xff));
	DM9000_iow(DM9000_EPCR, 0xa);	/* Issue phyxcer write command */
	udelay(500);			/* Wait write complete */
	DM9000_iow(DM9000_EPCR, 0x0);	/* Clear phyxcer write command */
	DM9000_DBG("dm9000_phy_write(reg:0x%x, value:0x%x)\n", reg, value);
}

int dm9000_initialize(bd_t *bis)
{
	struct eth_device *dev = &(dm9000_info.netdev);

	/* Load MAC address from EEPROM */
	dm9000_get_enetaddr(dev);

	dev->init = dm9000_init;
	dev->halt = dm9000_halt;
	dev->send = dm9000_send;
	dev->recv = dm9000_rx;
	strcpy(dev->name, "dm9000");

	eth_register(dev);

	return 0;
}
