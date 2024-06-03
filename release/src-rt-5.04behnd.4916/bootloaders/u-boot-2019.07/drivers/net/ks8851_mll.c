// SPDX-License-Identifier: GPL-2.0+
/*
 * Micrel KS8851_MLL 16bit Network driver
 * Copyright (c) 2011 Roberto Cerati <roberto.cerati@bticino.it>
 */

#include <asm/io.h>
#include <common.h>
#include <command.h>
#include <malloc.h>
#include <net.h>
#include <miiphy.h>

#include "ks8851_mll.h"

#define DRIVERNAME			"ks8851_mll"

#define MAX_RECV_FRAMES			32
#define MAX_BUF_SIZE			2048
#define TX_BUF_SIZE			2000
#define RX_BUF_SIZE			2000

static const struct chip_id chip_ids[] =  {
	{CIDER_ID, "KSZ8851"},
	{0, NULL},
};

/*
 * union ks_tx_hdr - tx header data
 * @txb: The header as bytes
 * @txw: The header as 16bit, little-endian words
 *
 * A dual representation of the tx header data to allow
 * access to individual bytes, and to allow 16bit accesses
 * with 16bit alignment.
 */
union ks_tx_hdr {
	u8      txb[4];
	__le16  txw[2];
};

/*
 * struct ks_net - KS8851 driver private data
 * @net_device	: The network device we're bound to
 * @txh		: temporaly buffer to save status/length.
 * @frame_head_info	: frame header information for multi-pkt rx.
 * @statelock	: Lock on this structure for tx list.
 * @msg_enable	: The message flags controlling driver output (see ethtool).
 * @frame_cnt	: number of frames received.
 * @bus_width	: i/o bus width.
 * @irq		: irq number assigned to this device.
 * @rc_rxqcr	: Cached copy of KS_RXQCR.
 * @rc_txcr	: Cached copy of KS_TXCR.
 * @rc_ier	: Cached copy of KS_IER.
 * @sharedbus	: Multipex(addr and data bus) mode indicator.
 * @cmd_reg_cache	: command register cached.
 * @cmd_reg_cache_int	: command register cached. Used in the irq handler.
 * @promiscuous	: promiscuous mode indicator.
 * @all_mcast	: mutlicast indicator.
 * @mcast_lst_size	: size of multicast list.
 * @mcast_lst		: multicast list.
 * @mcast_bits		: multicast enabed.
 * @mac_addr		: MAC address assigned to this device.
 * @fid			: frame id.
 * @extra_byte		: number of extra byte prepended rx pkt.
 * @enabled		: indicator this device works.
 */

/* Receive multiplex framer header info */
struct type_frame_head {
	u16	sts;         /* Frame status */
	u16	len;         /* Byte count */
} fr_h_i[MAX_RECV_FRAMES];

struct ks_net {
	struct net_device	*netdev;
	union ks_tx_hdr		txh;
	struct type_frame_head	*frame_head_info;
	u32			msg_enable;
	u32			frame_cnt;
	int			bus_width;
	int			irq;
	u16			rc_rxqcr;
	u16			rc_txcr;
	u16			rc_ier;
	u16			sharedbus;
	u16			cmd_reg_cache;
	u16			cmd_reg_cache_int;
	u16			promiscuous;
	u16			all_mcast;
	u16			mcast_lst_size;
	u8			mcast_lst[MAX_MCAST_LST][MAC_ADDR_LEN];
	u8			mcast_bits[HW_MCAST_SIZE];
	u8			mac_addr[6];
	u8                      fid;
	u8			extra_byte;
	u8			enabled;
} ks_str, *ks;

#define BE3             0x8000      /* Byte Enable 3 */
#define BE2             0x4000      /* Byte Enable 2 */
#define BE1             0x2000      /* Byte Enable 1 */
#define BE0             0x1000      /* Byte Enable 0 */

static u8 ks_rdreg8(struct eth_device *dev, u16 offset)
{
	u8 shift_bit = offset & 0x03;
	u8 shift_data = (offset & 1) << 3;

	writew(offset | (BE0 << shift_bit), dev->iobase + 2);

	return (u8)(readw(dev->iobase) >> shift_data);
}

static u16 ks_rdreg16(struct eth_device *dev, u16 offset)
{
	writew(offset | ((BE1 | BE0) << (offset & 0x02)), dev->iobase + 2);

	return readw(dev->iobase);
}

static void ks_wrreg8(struct eth_device *dev, u16 offset, u8 val)
{
	u8 shift_bit = (offset & 0x03);
	u16 value_write = (u16)(val << ((offset & 1) << 3));

	writew(offset | (BE0 << shift_bit), dev->iobase + 2);
	writew(value_write, dev->iobase);
}

static void ks_wrreg16(struct eth_device *dev, u16 offset, u16 val)
{
	writew(offset | ((BE1 | BE0) << (offset & 0x02)), dev->iobase + 2);
	writew(val, dev->iobase);
}

/*
 * ks_inblk - read a block of data from QMU. This is called after sudo DMA mode
 * enabled.
 * @ks: The chip state
 * @wptr: buffer address to save data
 * @len: length in byte to read
 */
static inline void ks_inblk(struct eth_device *dev, u16 *wptr, u32 len)
{
	len >>= 1;

	while (len--)
		*wptr++ = readw(dev->iobase);
}

/*
 * ks_outblk - write data to QMU. This is called after sudo DMA mode enabled.
 * @ks: The chip information
 * @wptr: buffer address
 * @len: length in byte to write
 */
static inline void ks_outblk(struct eth_device *dev, u16 *wptr, u32 len)
{
	len >>= 1;

	while (len--)
		writew(*wptr++, dev->iobase);
}

static void ks_enable_int(struct eth_device *dev)
{
	ks_wrreg16(dev, KS_IER, ks->rc_ier);
}

static void ks_set_powermode(struct eth_device *dev, unsigned pwrmode)
{
	unsigned pmecr;

	ks_rdreg16(dev, KS_GRR);
	pmecr = ks_rdreg16(dev, KS_PMECR);
	pmecr &= ~PMECR_PM_MASK;
	pmecr |= pwrmode;

	ks_wrreg16(dev, KS_PMECR, pmecr);
}

/*
 * ks_read_config - read chip configuration of bus width.
 * @ks: The chip information
 */
static void ks_read_config(struct eth_device *dev)
{
	u16 reg_data = 0;

	/* Regardless of bus width, 8 bit read should always work. */
	reg_data = ks_rdreg8(dev, KS_CCR) & 0x00FF;
	reg_data |= ks_rdreg8(dev, KS_CCR + 1) << 8;

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
static void ks_soft_reset(struct eth_device *dev, unsigned op)
{
	/* Disable interrupt first */
	ks_wrreg16(dev, KS_IER, 0x0000);
	ks_wrreg16(dev, KS_GRR, op);
	mdelay(10);	/* wait a short time to effect reset */
	ks_wrreg16(dev, KS_GRR, 0);
	mdelay(1);	/* wait for condition to clear */
}

void ks_enable_qmu(struct eth_device *dev)
{
	u16 w;

	w = ks_rdreg16(dev, KS_TXCR);

	/* Enables QMU Transmit (TXCR). */
	ks_wrreg16(dev, KS_TXCR, w | TXCR_TXE);

	/* Enable RX Frame Count Threshold and Auto-Dequeue RXQ Frame */
	w = ks_rdreg16(dev, KS_RXQCR);
	ks_wrreg16(dev, KS_RXQCR, w | RXQCR_RXFCTE);

	/* Enables QMU Receive (RXCR1). */
	w = ks_rdreg16(dev, KS_RXCR1);
	ks_wrreg16(dev, KS_RXCR1, w | RXCR1_RXE);
}

static void ks_disable_qmu(struct eth_device *dev)
{
	u16 w;

	w = ks_rdreg16(dev, KS_TXCR);

	/* Disables QMU Transmit (TXCR). */
	w &= ~TXCR_TXE;
	ks_wrreg16(dev, KS_TXCR, w);

	/* Disables QMU Receive (RXCR1). */
	w = ks_rdreg16(dev, KS_RXCR1);
	w &= ~RXCR1_RXE;
	ks_wrreg16(dev, KS_RXCR1, w);
}

static inline void ks_read_qmu(struct eth_device *dev, u16 *buf, u32 len)
{
	u32 r = ks->extra_byte & 0x1;
	u32 w = ks->extra_byte - r;

	/* 1. set sudo DMA mode */
	ks_wrreg16(dev, KS_RXFDPR, RXFDPR_RXFPAI);
	ks_wrreg8(dev, KS_RXQCR, (ks->rc_rxqcr | RXQCR_SDA) & 0xff);

	/*
	 * 2. read prepend data
	 *
	 * read 4 + extra bytes and discard them.
	 * extra bytes for dummy, 2 for status, 2 for len
	 */

	if (r)
		ks_rdreg8(dev, 0);

	ks_inblk(dev, buf, w + 2 + 2);

	/* 3. read pkt data */
	ks_inblk(dev, buf, ALIGN(len, 4));

	/* 4. reset sudo DMA Mode */
	ks_wrreg8(dev, KS_RXQCR, (ks->rc_rxqcr & ~RXQCR_SDA) & 0xff);
}

static void ks_rcv(struct eth_device *dev, uchar **pv_data)
{
	struct type_frame_head *frame_hdr = ks->frame_head_info;
	int i;

	ks->frame_cnt = ks_rdreg16(dev, KS_RXFCTR) >> 8;

	/* read all header information */
	for (i = 0; i < ks->frame_cnt; i++) {
		/* Checking Received packet status */
		frame_hdr->sts = ks_rdreg16(dev, KS_RXFHSR);
		/* Get packet len from hardware */
		frame_hdr->len = ks_rdreg16(dev, KS_RXFHBCR);
		frame_hdr++;
	}

	frame_hdr = ks->frame_head_info;
	while (ks->frame_cnt--) {
		if ((frame_hdr->sts & RXFSHR_RXFV) &&
		    (frame_hdr->len < RX_BUF_SIZE) &&
		    frame_hdr->len) {
			/* read data block including CRC 4 bytes */
			ks_read_qmu(dev, (u16 *)(*pv_data), frame_hdr->len);

			/* net_rx_packets buffer size is ok (*pv_data) */
			net_process_received_packet(*pv_data, frame_hdr->len);
			pv_data++;
		} else {
			ks_wrreg16(dev, KS_RXQCR, (ks->rc_rxqcr | RXQCR_RRXEF));
			printf(DRIVERNAME ": bad packet\n");
		}
		frame_hdr++;
	}
}

/*
 * ks_read_selftest - read the selftest memory info.
 * @ks: The device state
 *
 * Read and check the TX/RX memory selftest information.
 */
static int ks_read_selftest(struct eth_device *dev)
{
	u16 both_done = MBIR_TXMBF | MBIR_RXMBF;
	u16 mbir;
	int ret = 0;

	mbir = ks_rdreg16(dev, KS_MBIR);

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

static void ks_setup(struct eth_device *dev)
{
	u16 w;

	/* Setup Transmit Frame Data Pointer Auto-Increment (TXFDPR) */
	ks_wrreg16(dev, KS_TXFDPR, TXFDPR_TXFPAI);

	/* Setup Receive Frame Data Pointer Auto-Increment */
	ks_wrreg16(dev, KS_RXFDPR, RXFDPR_RXFPAI);

	/* Setup Receive Frame Threshold - 1 frame (RXFCTFC) */
	ks_wrreg16(dev, KS_RXFCTR, 1 & RXFCTR_THRESHOLD_MASK);

	/* Setup RxQ Command Control (RXQCR) */
	ks->rc_rxqcr = RXQCR_CMD_CNTL;
	ks_wrreg16(dev, KS_RXQCR, ks->rc_rxqcr);

	/*
	 * set the force mode to half duplex, default is full duplex
	 * because if the auto-negotiation fails, most switch uses
	 * half-duplex.
	 */
	w = ks_rdreg16(dev, KS_P1MBCR);
	w &= ~P1MBCR_FORCE_FDX;
	ks_wrreg16(dev, KS_P1MBCR, w);

	w = TXCR_TXFCE | TXCR_TXPE | TXCR_TXCRC | TXCR_TCGIP;
	ks_wrreg16(dev, KS_TXCR, w);

	w = RXCR1_RXFCE | RXCR1_RXBE | RXCR1_RXUE | RXCR1_RXME | RXCR1_RXIPFCC;

	/* Normal mode */
	w |= RXCR1_RXPAFMA;

	ks_wrreg16(dev, KS_RXCR1, w);
}

static void ks_setup_int(struct eth_device *dev)
{
	ks->rc_ier = 0x00;

	/* Clear the interrupts status of the hardware. */
	ks_wrreg16(dev, KS_ISR, 0xffff);

	/* Enables the interrupts of the hardware. */
	ks->rc_ier = (IRQ_LCI | IRQ_TXI | IRQ_RXI);
}

static int ks8851_mll_detect_chip(struct eth_device *dev)
{
	unsigned short val, i;

	ks_read_config(dev);

	val = ks_rdreg16(dev, KS_CIDER);

	if (val == 0xffff) {
		/* Special case -- no chip present */
		printf(DRIVERNAME ":  is chip mounted ?\n");
		return -1;
	} else if ((val & 0xfff0) != CIDER_ID) {
		printf(DRIVERNAME ": Invalid chip id 0x%04x\n", val);
		return -1;
	}

	debug("Read back KS8851 id 0x%x\n", val);

	/* only one entry in the table */
	val &= 0xfff0;
	for (i = 0; chip_ids[i].id != 0; i++) {
		if (chip_ids[i].id == val)
			break;
	}
	if (!chip_ids[i].id) {
		printf(DRIVERNAME ": Unknown chip ID %04x\n", val);
		return -1;
	}

	dev->priv = (void *)&chip_ids[i];

	return 0;
}

static void ks8851_mll_reset(struct eth_device *dev)
{
	/* wake up powermode to normal mode */
	ks_set_powermode(dev, PMECR_PM_NORMAL);
	mdelay(1);	/* wait for normal mode to take effect */

	/* Disable interrupt and reset */
	ks_soft_reset(dev, GRR_GSR);

	/* turn off the IRQs and ack any outstanding */
	ks_wrreg16(dev, KS_IER, 0x0000);
	ks_wrreg16(dev, KS_ISR, 0xffff);

	/* shutdown RX/TX QMU */
	ks_disable_qmu(dev);
}

static void ks8851_mll_phy_configure(struct eth_device *dev)
{
	u16 data;

	ks_setup(dev);
	ks_setup_int(dev);

	/* Probing the phy */
	data = ks_rdreg16(dev, KS_OBCR);
	ks_wrreg16(dev, KS_OBCR, data | OBCR_ODS_16MA);

	debug(DRIVERNAME ": phy initialized\n");
}

static void ks8851_mll_enable(struct eth_device *dev)
{
	ks_wrreg16(dev, KS_ISR, 0xffff);
	ks_enable_int(dev);
	ks_enable_qmu(dev);
}

static int ks8851_mll_init(struct eth_device *dev, bd_t *bd)
{
	struct chip_id *id = dev->priv;

	debug(DRIVERNAME ": detected %s controller\n", id->name);

	if (ks_read_selftest(dev)) {
		printf(DRIVERNAME ": Selftest failed\n");
		return -1;
	}

	ks8851_mll_reset(dev);

	/* Configure the PHY, initialize the link state */
	ks8851_mll_phy_configure(dev);

	/* static allocation of private informations */
	ks->frame_head_info = fr_h_i;

	/* Turn on Tx + Rx */
	ks8851_mll_enable(dev);

	return 0;
}

static void ks_write_qmu(struct eth_device *dev, u8 *pdata, u16 len)
{
	/* start header at txb[0] to align txw entries */
	ks->txh.txw[0] = 0;
	ks->txh.txw[1] = cpu_to_le16(len);

	/* 1. set sudo-DMA mode */
	ks_wrreg16(dev, KS_TXFDPR, TXFDPR_TXFPAI);
	ks_wrreg8(dev, KS_RXQCR, (ks->rc_rxqcr | RXQCR_SDA) & 0xff);
	/* 2. write status/lenth info */
	ks_outblk(dev, ks->txh.txw, 4);
	/* 3. write pkt data */
	ks_outblk(dev, (u16 *)pdata, ALIGN(len, 4));
	/* 4. reset sudo-DMA mode */
	ks_wrreg8(dev, KS_RXQCR, (ks->rc_rxqcr & ~RXQCR_SDA) & 0xff);
	/* 5. Enqueue Tx(move the pkt from TX buffer into TXQ) */
	ks_wrreg16(dev, KS_TXQCR, TXQCR_METFE);
	/* 6. wait until TXQCR_METFE is auto-cleared */
	do { } while (ks_rdreg16(dev, KS_TXQCR) & TXQCR_METFE);
}

static int ks8851_mll_send(struct eth_device *dev, void *packet, int length)
{
	u8 *data = (u8 *)packet;
	u16 tmplen = (u16)length;
	u16 retv;

	/*
	 * Extra space are required:
	 * 4 byte for alignment, 4 for status/length, 4 for CRC
	 */
	retv = ks_rdreg16(dev, KS_TXMIR) & 0x1fff;
	if (retv >= tmplen + 12) {
		ks_write_qmu(dev, data, tmplen);
		return 0;
	} else {
		printf(DRIVERNAME ": failed to send packet: No buffer\n");
		return -1;
	}
}

static void ks8851_mll_halt(struct eth_device *dev)
{
	ks8851_mll_reset(dev);
}

/*
 * Maximum receive ring size; that is, the number of packets
 * we can buffer before overflow happens. Basically, this just
 * needs to be enough to prevent a packet being discarded while
 * we are processing the previous one.
 */
static int ks8851_mll_recv(struct eth_device *dev)
{
	u16 status;

	status = ks_rdreg16(dev, KS_ISR);

	ks_wrreg16(dev, KS_ISR, status);

	if ((status & IRQ_RXI))
		ks_rcv(dev, (uchar **)net_rx_packets);

	if ((status & IRQ_LDI)) {
		u16 pmecr = ks_rdreg16(dev, KS_PMECR);
		pmecr &= ~PMECR_WKEVT_MASK;
		ks_wrreg16(dev, KS_PMECR, pmecr | PMECR_WKEVT_LINK);
	}

	return 0;
}

static int ks8851_mll_write_hwaddr(struct eth_device *dev)
{
	u16 addrl, addrm, addrh;

	addrh = (dev->enetaddr[0] << 8) | dev->enetaddr[1];
	addrm = (dev->enetaddr[2] << 8) | dev->enetaddr[3];
	addrl = (dev->enetaddr[4] << 8) | dev->enetaddr[5];

	ks_wrreg16(dev, KS_MARH, addrh);
	ks_wrreg16(dev, KS_MARM, addrm);
	ks_wrreg16(dev, KS_MARL, addrl);

	return 0;
}

int ks8851_mll_initialize(u8 dev_num, int base_addr)
{
	struct eth_device *dev;

	dev = malloc(sizeof(*dev));
	if (!dev) {
		printf("Error: Failed to allocate memory\n");
		return -1;
	}
	memset(dev, 0, sizeof(*dev));

	dev->iobase = base_addr;

	ks = &ks_str;

	/* Try to detect chip. Will fail if not present. */
	if (ks8851_mll_detect_chip(dev)) {
		free(dev);
		return -1;
	}

	dev->init = ks8851_mll_init;
	dev->halt = ks8851_mll_halt;
	dev->send = ks8851_mll_send;
	dev->recv = ks8851_mll_recv;
	dev->write_hwaddr = ks8851_mll_write_hwaddr;
	sprintf(dev->name, "%s-%hu", DRIVERNAME, dev_num);

	eth_register(dev);

	return 0;
}
