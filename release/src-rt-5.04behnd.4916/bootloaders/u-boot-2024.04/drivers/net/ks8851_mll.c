// SPDX-License-Identifier: GPL-2.0+
/*
 * Micrel KS8851_MLL 16bit Network driver
 * Copyright (c) 2011 Roberto Cerati <roberto.cerati@bticino.it>
 */

#include <log.h>
#include <asm/io.h>
#include <common.h>
#include <command.h>
#include <malloc.h>
#include <net.h>
#include <miiphy.h>
#include <linux/delay.h>

#include "ks8851_mll.h"

#define DRIVERNAME			"ks8851_mll"

#define RX_BUF_SIZE			2000

/*
 * struct ks_net - KS8851 driver private data
 * @dev		: legacy non-DM ethernet device structure
 * @iobase	: register base
 * @bus_width	: i/o bus width.
 * @sharedbus	: Multipex(addr and data bus) mode indicator.
 * @extra_byte	: number of extra byte prepended rx pkt.
 */
struct ks_net {
	phys_addr_t		iobase;
	int			bus_width;
	u16			sharedbus;
	u16			rxfc;
	u8			extra_byte;
};

#define BE3             0x8000      /* Byte Enable 3 */
#define BE2             0x4000      /* Byte Enable 2 */
#define BE1             0x2000      /* Byte Enable 1 */
#define BE0             0x1000      /* Byte Enable 0 */

static u8 ks_rdreg8(struct ks_net *ks, u16 offset)
{
	u8 shift_bit = offset & 0x03;
	u8 shift_data = (offset & 1) << 3;

	writew(offset | (BE0 << shift_bit), ks->iobase + 2);

	return (u8)(readw(ks->iobase) >> shift_data);
}

static u16 ks_rdreg16(struct ks_net *ks, u16 offset)
{
	writew(offset | ((BE1 | BE0) << (offset & 0x02)), ks->iobase + 2);

	return readw(ks->iobase);
}

static void ks_wrreg16(struct ks_net *ks, u16 offset, u16 val)
{
	writew(offset | ((BE1 | BE0) << (offset & 0x02)), ks->iobase + 2);
	writew(val, ks->iobase);
}

/*
 * ks_inblk - read a block of data from QMU. This is called after sudo DMA mode
 * enabled.
 * @ks: The chip state
 * @wptr: buffer address to save data
 * @len: length in byte to read
 */
static inline void ks_inblk(struct ks_net *ks, u16 *wptr, u32 len)
{
	len >>= 1;

	while (len--)
		*wptr++ = readw(ks->iobase);
}

/*
 * ks_outblk - write data to QMU. This is called after sudo DMA mode enabled.
 * @ks: The chip information
 * @wptr: buffer address
 * @len: length in byte to write
 */
static inline void ks_outblk(struct ks_net *ks, u16 *wptr, u32 len)
{
	len >>= 1;

	while (len--)
		writew(*wptr++, ks->iobase);
}

static void ks_enable_int(struct ks_net *ks)
{
	ks_wrreg16(ks, KS_IER, IRQ_LCI | IRQ_TXI | IRQ_RXI);
}

static void ks_set_powermode(struct ks_net *ks, unsigned int pwrmode)
{
	unsigned int pmecr;

	ks_rdreg16(ks, KS_GRR);
	pmecr = ks_rdreg16(ks, KS_PMECR);
	pmecr &= ~PMECR_PM_MASK;
	pmecr |= pwrmode;

	ks_wrreg16(ks, KS_PMECR, pmecr);
}

/*
 * ks_read_config - read chip configuration of bus width.
 * @ks: The chip information
 */
static void ks_read_config(struct ks_net *ks)
{
	u16 reg_data = 0;

	/* Regardless of bus width, 8 bit read should always work. */
	reg_data = ks_rdreg8(ks, KS_CCR) & 0x00FF;
	reg_data |= ks_rdreg8(ks, KS_CCR + 1) << 8;

	/* addr/data bus are multiplexed */
	ks->sharedbus = (reg_data & CCR_SHARED) == CCR_SHARED;

	/*
	 * There are garbage data when reading data from QMU,
	 * depending on bus-width.
	 */
	if (reg_data & CCR_8BIT) {
		ks->bus_width = ENUM_BUS_8BIT;
		ks->extra_byte = 1;
	} else if (reg_data & CCR_16BIT) {
		ks->bus_width = ENUM_BUS_16BIT;
		ks->extra_byte = 2;
	} else {
		ks->bus_width = ENUM_BUS_32BIT;
		ks->extra_byte = 4;
	}
}

/*
 * ks_soft_reset - issue one of the soft reset to the device
 * @ks: The device state.
 * @op: The bit(s) to set in the GRR
 *
 * Issue the relevant soft-reset command to the device's GRR register
 * specified by @op.
 *
 * Note, the delays are in there as a caution to ensure that the reset
 * has time to take effect and then complete. Since the datasheet does
 * not currently specify the exact sequence, we have chosen something
 * that seems to work with our device.
 */
static void ks_soft_reset(struct ks_net *ks, unsigned int op)
{
	/* Disable interrupt first */
	ks_wrreg16(ks, KS_IER, 0x0000);
	ks_wrreg16(ks, KS_GRR, op);
	mdelay(10);	/* wait a short time to effect reset */
	ks_wrreg16(ks, KS_GRR, 0);
	mdelay(1);	/* wait for condition to clear */
}

void ks_enable_qmu(struct ks_net *ks)
{
	u16 w;

	w = ks_rdreg16(ks, KS_TXCR);

	/* Enables QMU Transmit (TXCR). */
	ks_wrreg16(ks, KS_TXCR, w | TXCR_TXE);

	/* Enable RX Frame Count Threshold and Auto-Dequeue RXQ Frame */
	w = ks_rdreg16(ks, KS_RXQCR);
	ks_wrreg16(ks, KS_RXQCR, w | RXQCR_RXFCTE);

	/* Enables QMU Receive (RXCR1). */
	w = ks_rdreg16(ks, KS_RXCR1);
	ks_wrreg16(ks, KS_RXCR1, w | RXCR1_RXE);
}

static void ks_disable_qmu(struct ks_net *ks)
{
	u16 w;

	w = ks_rdreg16(ks, KS_TXCR);

	/* Disables QMU Transmit (TXCR). */
	w &= ~TXCR_TXE;
	ks_wrreg16(ks, KS_TXCR, w);

	/* Disables QMU Receive (RXCR1). */
	w = ks_rdreg16(ks, KS_RXCR1);
	w &= ~RXCR1_RXE;
	ks_wrreg16(ks, KS_RXCR1, w);
}

static inline void ks_read_qmu(struct ks_net *ks, u16 *buf, u32 len)
{
	u32 r = ks->extra_byte & 0x1;
	u32 w = ks->extra_byte - r;

	/* 1. set sudo DMA mode */
	ks_wrreg16(ks, KS_RXFDPR, RXFDPR_RXFPAI);
	ks_wrreg16(ks, KS_RXQCR, RXQCR_CMD_CNTL | RXQCR_SDA);

	/*
	 * 2. read prepend data
	 *
	 * read 4 + extra bytes and discard them.
	 * extra bytes for dummy, 2 for status, 2 for len
	 */

	if (r)
		ks_rdreg8(ks, 0);

	ks_inblk(ks, buf, w + 2 + 2);

	/* 3. read pkt data */
	ks_inblk(ks, buf, ALIGN(len, 4));

	/* 4. reset sudo DMA Mode */
	ks_wrreg16(ks, KS_RXQCR, RXQCR_CMD_CNTL);
}

static int ks_rcv(struct ks_net *ks, uchar *data)
{
	u16 sts, len;

	if (!ks->rxfc)
		ks->rxfc = ks_rdreg16(ks, KS_RXFCTR) >> 8;

	if (!ks->rxfc)
		return 0;

	/* Checking Received packet status */
	sts = ks_rdreg16(ks, KS_RXFHSR);
	/* Get packet len from hardware */
	len = ks_rdreg16(ks, KS_RXFHBCR);

	if ((sts & RXFSHR_RXFV) && len && (len < RX_BUF_SIZE)) {
		/* read data block including CRC 4 bytes */
		ks_read_qmu(ks, (u16 *)data, len);
		ks->rxfc--;
		return len - 4;
	}

	ks_wrreg16(ks, KS_RXQCR, RXQCR_CMD_CNTL | RXQCR_RRXEF);
	printf(DRIVERNAME ": bad packet (sts=0x%04x len=0x%04x)\n", sts, len);
	ks->rxfc = 0;
	return 0;
}

/*
 * ks_read_selftest - read the selftest memory info.
 * @ks: The device state
 *
 * Read and check the TX/RX memory selftest information.
 */
static int ks_read_selftest(struct ks_net *ks)
{
	u16 both_done = MBIR_TXMBF | MBIR_RXMBF;
	u16 mbir;
	int ret = 0;

	mbir = ks_rdreg16(ks, KS_MBIR);

	if ((mbir & both_done) != both_done) {
		printf(DRIVERNAME ": Memory selftest not finished\n");
		return 0;
	}

	if (mbir & MBIR_TXMBFA) {
		printf(DRIVERNAME ": TX memory selftest fails\n");
		ret |= 1;
	}

	if (mbir & MBIR_RXMBFA) {
		printf(DRIVERNAME ": RX memory selftest fails\n");
		ret |= 2;
	}

	debug(DRIVERNAME ": the selftest passes\n");

	return ret;
}

static void ks_setup(struct ks_net *ks)
{
	u16 w;

	/* Setup Transmit Frame Data Pointer Auto-Increment (TXFDPR) */
	ks_wrreg16(ks, KS_TXFDPR, TXFDPR_TXFPAI);

	/* Setup Receive Frame Data Pointer Auto-Increment */
	ks_wrreg16(ks, KS_RXFDPR, RXFDPR_RXFPAI);

	/* Setup Receive Frame Threshold - 1 frame (RXFCTFC) */
	ks_wrreg16(ks, KS_RXFCTR, 1 & RXFCTR_THRESHOLD_MASK);

	/* Setup RxQ Command Control (RXQCR) */
	ks_wrreg16(ks, KS_RXQCR, RXQCR_CMD_CNTL);

	/*
	 * set the force mode to half duplex, default is full duplex
	 * because if the auto-negotiation fails, most switch uses
	 * half-duplex.
	 */
	w = ks_rdreg16(ks, KS_P1MBCR);
	w &= ~P1MBCR_FORCE_FDX;
	ks_wrreg16(ks, KS_P1MBCR, w);

	w = TXCR_TXFCE | TXCR_TXPE | TXCR_TXCRC | TXCR_TCGIP;
	ks_wrreg16(ks, KS_TXCR, w);

	w = RXCR1_RXFCE | RXCR1_RXBE | RXCR1_RXUE | RXCR1_RXME | RXCR1_RXIPFCC;

	/* Normal mode */
	w |= RXCR1_RXPAFMA;

	ks_wrreg16(ks, KS_RXCR1, w);
}

static void ks_setup_int(struct ks_net *ks)
{
	/* Clear the interrupts status of the hardware. */
	ks_wrreg16(ks, KS_ISR, 0xffff);
}

static int ks8851_mll_detect_chip(struct ks_net *ks)
{
	unsigned short val;

	ks_read_config(ks);

	val = ks_rdreg16(ks, KS_CIDER);

	if (val == 0xffff) {
		/* Special case -- no chip present */
		printf(DRIVERNAME ":  is chip mounted ?\n");
		return -1;
	} else if ((val & 0xfff0) != CIDER_ID) {
		printf(DRIVERNAME ": Invalid chip id 0x%04x\n", val);
		return -1;
	}

	debug("Read back KS8851 id 0x%x\n", val);

	if ((val & 0xfff0) != CIDER_ID) {
		printf(DRIVERNAME ": Unknown chip ID %04x\n", val);
		return -1;
	}

	return 0;
}

static void ks8851_mll_reset(struct ks_net *ks)
{
	/* wake up powermode to normal mode */
	ks_set_powermode(ks, PMECR_PM_NORMAL);
	mdelay(1);	/* wait for normal mode to take effect */

	/* Disable interrupt and reset */
	ks_soft_reset(ks, GRR_GSR);

	/* turn off the IRQs and ack any outstanding */
	ks_wrreg16(ks, KS_IER, 0x0000);
	ks_wrreg16(ks, KS_ISR, 0xffff);

	/* shutdown RX/TX QMU */
	ks_disable_qmu(ks);
}

static void ks8851_mll_phy_configure(struct ks_net *ks)
{
	u16 data;

	ks_setup(ks);
	ks_setup_int(ks);

	/* Probing the phy */
	data = ks_rdreg16(ks, KS_OBCR);
	ks_wrreg16(ks, KS_OBCR, data | OBCR_ODS_16MA);

	debug(DRIVERNAME ": phy initialized\n");
}

static void ks8851_mll_enable(struct ks_net *ks)
{
	ks_wrreg16(ks, KS_ISR, 0xffff);
	ks_enable_int(ks);
	ks_enable_qmu(ks);
}

static int ks8851_mll_init_common(struct ks_net *ks)
{
	if (ks_read_selftest(ks)) {
		printf(DRIVERNAME ": Selftest failed\n");
		return -1;
	}

	ks8851_mll_reset(ks);

	/* Configure the PHY, initialize the link state */
	ks8851_mll_phy_configure(ks);

	ks->rxfc = 0;

	/* Turn on Tx + Rx */
	ks8851_mll_enable(ks);

	return 0;
}

static void ks_write_qmu(struct ks_net *ks, u8 *pdata, u16 len)
{
	__le16 txw[2];
	/* start header at txb[0] to align txw entries */
	txw[0] = 0;
	txw[1] = cpu_to_le16(len);

	/* 1. set sudo-DMA mode */
	ks_wrreg16(ks, KS_TXFDPR, TXFDPR_TXFPAI);
	ks_wrreg16(ks, KS_RXQCR, RXQCR_CMD_CNTL | RXQCR_SDA);
	/* 2. write status/length info */
	ks_outblk(ks, txw, 4);
	/* 3. write pkt data */
	ks_outblk(ks, (u16 *)pdata, ALIGN(len, 4));
	/* 4. reset sudo-DMA mode */
	ks_wrreg16(ks, KS_RXQCR, RXQCR_CMD_CNTL);
	/* 5. Enqueue Tx(move the pkt from TX buffer into TXQ) */
	ks_wrreg16(ks, KS_TXQCR, TXQCR_METFE);
	/* 6. wait until TXQCR_METFE is auto-cleared */
	do { } while (ks_rdreg16(ks, KS_TXQCR) & TXQCR_METFE);
}

static int ks8851_mll_send_common(struct ks_net *ks, void *packet, int length)
{
	u8 *data = (u8 *)packet;
	u16 tmplen = (u16)length;
	u16 retv;

	/*
	 * Extra space are required:
	 * 4 byte for alignment, 4 for status/length, 4 for CRC
	 */
	retv = ks_rdreg16(ks, KS_TXMIR) & 0x1fff;
	if (retv >= tmplen + 12) {
		ks_write_qmu(ks, data, tmplen);
		return 0;
	}

	printf(DRIVERNAME ": failed to send packet: No buffer\n");
	return -1;
}

static void ks8851_mll_halt_common(struct ks_net *ks)
{
	ks8851_mll_reset(ks);
}

/*
 * Maximum receive ring size; that is, the number of packets
 * we can buffer before overflow happens. Basically, this just
 * needs to be enough to prevent a packet being discarded while
 * we are processing the previous one.
 */
static int ks8851_mll_recv_common(struct ks_net *ks, uchar *data)
{
	u16 status;
	int ret = 0;

	status = ks_rdreg16(ks, KS_ISR);

	ks_wrreg16(ks, KS_ISR, status);

	if (ks->rxfc || (status & IRQ_RXI))
		ret = ks_rcv(ks, data);

	if (status & IRQ_LDI) {
		u16 pmecr = ks_rdreg16(ks, KS_PMECR);

		pmecr &= ~PMECR_WKEVT_MASK;
		ks_wrreg16(ks, KS_PMECR, pmecr | PMECR_WKEVT_LINK);
	}

	return ret;
}

static void ks8851_mll_write_hwaddr_common(struct ks_net *ks, u8 enetaddr[6])
{
	u16 addrl, addrm, addrh;

	addrh = (enetaddr[0] << 8) | enetaddr[1];
	addrm = (enetaddr[2] << 8) | enetaddr[3];
	addrl = (enetaddr[4] << 8) | enetaddr[5];

	ks_wrreg16(ks, KS_MARH, addrh);
	ks_wrreg16(ks, KS_MARM, addrm);
	ks_wrreg16(ks, KS_MARL, addrl);
}

static int ks8851_start(struct udevice *dev)
{
	struct ks_net *ks = dev_get_priv(dev);

	return ks8851_mll_init_common(ks);
}

static void ks8851_stop(struct udevice *dev)
{
	struct ks_net *ks = dev_get_priv(dev);

	ks8851_mll_halt_common(ks);
}

static int ks8851_send(struct udevice *dev, void *packet, int length)
{
	struct ks_net *ks = dev_get_priv(dev);
	int ret;

	ret = ks8851_mll_send_common(ks, packet, length);

	return ret ? 0 : -ETIMEDOUT;
}

static int ks8851_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct ks_net *ks = dev_get_priv(dev);
	uchar *data = net_rx_packets[0];
	int ret;

	ret = ks8851_mll_recv_common(ks, data);
	if (ret)
		*packetp = (void *)data;

	return ret ? ret : -EAGAIN;
}

static int ks8851_write_hwaddr(struct udevice *dev)
{
	struct ks_net *ks = dev_get_priv(dev);
	struct eth_pdata *pdata = dev_get_plat(dev);

	ks8851_mll_write_hwaddr_common(ks, pdata->enetaddr);

	return 0;
}

static int ks8851_read_rom_hwaddr(struct udevice *dev)
{
	struct ks_net *ks = dev_get_priv(dev);
	struct eth_pdata *pdata = dev_get_plat(dev);
	u16 addrl, addrm, addrh;

	/* No EEPROM means no valid MAC address. */
	if (!(ks_rdreg16(ks, KS_CCR) & CCR_EEPROM))
		return -EINVAL;

	/*
	 * If the EEPROM contains valid MAC address, it is loaded into
	 * the NIC on power on. Read the MAC out of the NIC registers.
	 */
	addrl = ks_rdreg16(ks, KS_MARL);
	addrm = ks_rdreg16(ks, KS_MARM);
	addrh = ks_rdreg16(ks, KS_MARH);

	pdata->enetaddr[0] = (addrh >> 8) & 0xff;
	pdata->enetaddr[1] = addrh & 0xff;
	pdata->enetaddr[2] = (addrm >> 8) & 0xff;
	pdata->enetaddr[3] = addrm & 0xff;
	pdata->enetaddr[4] = (addrl >> 8) & 0xff;
	pdata->enetaddr[5] = addrl & 0xff;

	return !is_valid_ethaddr(pdata->enetaddr);
}

static int ks8851_bind(struct udevice *dev)
{
	return device_set_name(dev, dev->name);
}

static int ks8851_probe(struct udevice *dev)
{
	struct ks_net *ks = dev_get_priv(dev);

	/* Try to detect chip. Will fail if not present. */
	ks8851_mll_detect_chip(ks);

	return 0;
}

static int ks8851_of_to_plat(struct udevice *dev)
{
	struct ks_net *ks = dev_get_priv(dev);
	struct eth_pdata *pdata = dev_get_plat(dev);

	pdata->iobase = dev_read_addr(dev);
	ks->iobase = pdata->iobase;

	return 0;
}

static const struct eth_ops ks8851_ops = {
	.start		= ks8851_start,
	.stop		= ks8851_stop,
	.send		= ks8851_send,
	.recv		= ks8851_recv,
	.write_hwaddr	= ks8851_write_hwaddr,
	.read_rom_hwaddr = ks8851_read_rom_hwaddr,
};

static const struct udevice_id ks8851_ids[] = {
	{ .compatible = "micrel,ks8851-mll" },
	{ }
};

U_BOOT_DRIVER(ks8851) = {
	.name		= "eth_ks8851",
	.id		= UCLASS_ETH,
	.of_match	= ks8851_ids,
	.bind		= ks8851_bind,
	.of_to_plat = ks8851_of_to_plat,
	.probe		= ks8851_probe,
	.ops		= &ks8851_ops,
	.priv_auto	= sizeof(struct ks_net),
	.plat_auto	= sizeof(struct eth_pdata),
	.flags		= DM_FLAG_ALLOC_PRIV_DMA,
};
