// SPDX-License-Identifier: GPL-2.0
/*
 * rtl8139.c : U-Boot driver for the RealTek RTL8139
 *
 * Masami Komiya (mkomiya@sonare.it)
 *
 * Most part is taken from rtl8139.c of etherboot
 *
 */

/* rtl8139.c - etherboot driver for the Realtek 8139 chipset
 *
 * ported from the linux driver written by Donald Becker
 * by Rainer Bawidamann (Rainer.Bawidamann@informatik.uni-ulm.de) 1999
 *
 * changes to the original driver:
 * - removed support for interrupts, switching to polling mode (yuck!)
 * - removed support for the 8129 chip (external MII)
 */

/*********************************************************************/
/* Revision History						     */
/*********************************************************************/

/*
 * 28 Dec 2002 ken_yap@users.sourceforge.net (Ken Yap)
 *    Put in virt_to_bus calls to allow Etherboot relocation.
 *
 * 06 Apr 2001 ken_yap@users.sourceforge.net (Ken Yap)
 *    Following email from Hyun-Joon Cha, added a disable routine, otherwise
 *    NIC remains live and can crash the kernel later.
 *
 * 4 Feb 2000 espenlaub@informatik.uni-ulm.de (Klaus Espenlaub)
 *    Shuffled things around, removed the leftovers from the 8129 support
 *    that was in the Linux driver and added a bit more 8139 definitions.
 *    Moved the 8K receive buffer to a fixed, available address outside the
 *    0x98000-0x9ffff range. This is a bit of a hack, but currently the only
 *    way to make room for the Etherboot features that need substantial amounts
 *    of code like the ANSI console support. Currently the buffer is just below
 *    0x10000, so this even conforms to the tagged boot image specification,
 *    which reserves the ranges 0x00000-0x10000 and 0x98000-0xA0000. My
 *    interpretation of this "reserved" is that Etherboot may do whatever it
 *    likes, as long as its environment is kept intact (like the BIOS
 *    variables). Hopefully fixed rtl8139_recv() once and for all. The symptoms
 *    were that if Etherboot was left at the boot menu for several minutes, the
 *    first eth_poll failed. Seems like I am the only person who does this.
 *    First of all I fixed the debugging code and then set out for a long bug
 *    hunting session. It took me about a week full time work - poking around
 *    various places in the driver, reading Don Becker's and Jeff Garzik's Linux
 *    driver and even the FreeBSD driver (what a piece of crap!) - and
 *    eventually spotted the nasty thing: the transmit routine was acknowledging
 *    each and every interrupt pending, including the RxOverrun and RxFIFIOver
 *    interrupts. This confused the RTL8139 thoroughly. It destroyed the
 *    Rx ring contents by dumping the 2K FIFO contents right where we wanted to
 *    get the next packet. Oh well, what fun.
 *
 * 18 Jan 2000 mdc@thinguin.org (Marty Connor)
 *    Drastically simplified error handling. Basically, if any error
 *    in transmission or reception occurs, the card is reset.
 *    Also, pointed all transmit descriptors to the same buffer to
 *    save buffer space. This should decrease driver size and avoid
 *    corruption because of exceeding 32K during runtime.
 *
 * 28 Jul 1999 (Matthias Meixner - meixner@rbg.informatik.tu-darmstadt.de)
 *    rtl8139_recv was quite broken: it used the RxOK interrupt flag instead
 *    of the RxBufferEmpty flag which often resulted in very bad
 *    transmission performace - below 1kBytes/s.
 *
 */

#include <common.h>
#include <cpu_func.h>
#include <dm.h>
#include <log.h>
#include <malloc.h>
#include <net.h>
#include <netdev.h>
#include <asm/io.h>
#include <pci.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/types.h>

#define RTL_TIMEOUT	100000

/* PCI Tuning Parameters */
/* Threshold is bytes transferred to chip before transmission starts. */
#define TX_FIFO_THRESH 256	/* In bytes, rounded down to 32 byte units. */
#define RX_FIFO_THRESH	4	/* Rx buffer level before first PCI xfer.  */
#define RX_DMA_BURST	4	/* Maximum PCI burst, '4' is 256 bytes */
#define TX_DMA_BURST	4	/* Calculate as 16<<val. */
#define NUM_TX_DESC	4	/* Number of Tx descriptor registers. */
#define TX_BUF_SIZE	ETH_FRAME_LEN	/* FCS is added by the chip */
#define RX_BUF_LEN_IDX 0	/* 0, 1, 2 is allowed - 8,16,32K rx buffer */
#define RX_BUF_LEN (8192 << RX_BUF_LEN_IDX)

#define DEBUG_TX	0	/* set to 1 to enable debug code */
#define DEBUG_RX	0	/* set to 1 to enable debug code */

#define bus_to_phys(devno, a)	dm_pci_mem_to_phys((devno), (a))
#define phys_to_bus(devno, a)	dm_pci_phys_to_mem((devno), (a))

/* Symbolic offsets to registers. */
/* Ethernet hardware address. */
#define RTL_REG_MAC0				0x00
/* Multicast filter. */
#define RTL_REG_MAR0				0x08
/* Transmit status (four 32bit registers). */
#define RTL_REG_TXSTATUS0			0x10
/* Tx descriptors (also four 32bit). */
#define RTL_REG_TXADDR0				0x20
#define RTL_REG_RXBUF				0x30
#define RTL_REG_RXEARLYCNT			0x34
#define RTL_REG_RXEARLYSTATUS			0x36
#define RTL_REG_CHIPCMD				0x37
#define RTL_REG_CHIPCMD_CMDRESET		BIT(4)
#define RTL_REG_CHIPCMD_CMDRXENB		BIT(3)
#define RTL_REG_CHIPCMD_CMDTXENB		BIT(2)
#define RTL_REG_CHIPCMD_RXBUFEMPTY		BIT(0)
#define RTL_REG_RXBUFPTR			0x38
#define RTL_REG_RXBUFADDR			0x3A
#define RTL_REG_INTRMASK			0x3C
#define RTL_REG_INTRSTATUS			0x3E
#define RTL_REG_INTRSTATUS_PCIERR		BIT(15)
#define RTL_REG_INTRSTATUS_PCSTIMEOUT		BIT(14)
#define RTL_REG_INTRSTATUS_CABLELENCHANGE	BIT(13)
#define RTL_REG_INTRSTATUS_RXFIFOOVER		BIT(6)
#define RTL_REG_INTRSTATUS_RXUNDERRUN		BIT(5)
#define RTL_REG_INTRSTATUS_RXOVERFLOW		BIT(4)
#define RTL_REG_INTRSTATUS_TXERR		BIT(3)
#define RTL_REG_INTRSTATUS_TXOK			BIT(2)
#define RTL_REG_INTRSTATUS_RXERR		BIT(1)
#define RTL_REG_INTRSTATUS_RXOK			BIT(0)
#define RTL_REG_TXCONFIG			0x40
#define RTL_REG_RXCONFIG			0x44
#define RTL_REG_RXCONFIG_RXCFGWRAP		BIT(7)
#define RTL_REG_RXCONFIG_ACCEPTERR		BIT(5)
#define RTL_REG_RXCONFIG_ACCEPTRUNT		BIT(4)
#define RTL_REG_RXCONFIG_ACCEPTBROADCAST	BIT(3)
#define RTL_REG_RXCONFIG_ACCEPTMULTICAST	BIT(2)
#define RTL_REG_RXCONFIG_ACCEPTMYPHYS		BIT(1)
#define RTL_REG_RXCONFIG_ACCEPTALLPHYS		BIT(0)
/* general-purpose counter. */
#define RTL_REG_TIMER				0x48
/* 24 bits valid, write clears. */
#define RTL_REG_RXMISSED			0x4C
#define RTL_REG_CFG9346				0x50
#define RTL_REG_CONFIG0				0x51
#define RTL_REG_CONFIG1				0x52
/* intr if gp counter reaches this value */
#define RTL_REG_TIMERINTRREG			0x54
#define RTL_REG_MEDIASTATUS			0x58
#define RTL_REG_MEDIASTATUS_MSRTXFLOWENABLE	BIT(7)
#define RTL_REG_MEDIASTATUS_MSRRXFLOWENABLE	BIT(6)
#define RTL_REG_MEDIASTATUS_MSRSPEED10		BIT(3)
#define RTL_REG_MEDIASTATUS_MSRLINKFAIL		BIT(2)
#define RTL_REG_MEDIASTATUS_MSRRXPAUSEFLAG	BIT(1)
#define RTL_REG_MEDIASTATUS_MSRTXPAUSEFLAG	BIT(0)
#define RTL_REG_CONFIG3				0x59
#define RTL_REG_MULTIINTR			0x5C
/* revision of the RTL8139 chip */
#define RTL_REG_REVISIONID			0x5E
#define RTL_REG_TXSUMMARY			0x60
#define RTL_REG_MII_BMCR			0x62
#define RTL_REG_MII_BMSR			0x64
#define RTL_REG_NWAYADVERT			0x66
#define RTL_REG_NWAYLPAR			0x68
#define RTL_REG_NWAYEXPANSION			0x6A
#define RTL_REG_DISCONNECTCNT			0x6C
#define RTL_REG_FALSECARRIERCNT			0x6E
#define RTL_REG_NWAYTESTREG			0x70
/* packet received counter */
#define RTL_REG_RXCNT				0x72
/* chip status and configuration register */
#define RTL_REG_CSCR				0x74
#define RTL_REG_PHYPARM1			0x78
#define RTL_REG_TWISTERPARM			0x7c
/* undocumented */
#define RTL_REG_PHYPARM2			0x80
/*
 * from 0x84 onwards are a number of power management/wakeup frame
 * definitions we will probably never need to know about.
 */

#define RTL_STS_RXMULTICAST			BIT(15)
#define RTL_STS_RXPHYSICAL			BIT(14)
#define RTL_STS_RXBROADCAST			BIT(13)
#define RTL_STS_RXBADSYMBOL			BIT(5)
#define RTL_STS_RXRUNT				BIT(4)
#define RTL_STS_RXTOOLONG			BIT(3)
#define RTL_STS_RXCRCERR			BIT(2)
#define RTL_STS_RXBADALIGN			BIT(1)
#define RTL_STS_RXSTATUSOK			BIT(0)

struct rtl8139_priv {
	struct udevice		*devno;
	unsigned int		rxstatus;
	unsigned int		cur_rx;
	unsigned int		cur_tx;
	unsigned long		ioaddr;
	unsigned char		enetaddr[6];
};

/* The RTL8139 can only transmit from a contiguous, aligned memory block.  */
static unsigned char tx_buffer[TX_BUF_SIZE] __aligned(4);
static unsigned char rx_ring[RX_BUF_LEN + 16] __aligned(4);

/* Serial EEPROM section. */

/*  EEPROM_Ctrl bits. */
#define EE_SHIFT_CLK	0x04	/* EEPROM shift clock. */
#define EE_CS		0x08	/* EEPROM chip select. */
#define EE_DATA_WRITE	0x02	/* EEPROM chip data in. */
#define EE_WRITE_0	0x00
#define EE_WRITE_1	0x02
#define EE_DATA_READ	0x01	/* EEPROM chip data out. */
#define EE_ENB		(0x80 | EE_CS)

/* The EEPROM commands include the alway-set leading bit. */
#define EE_WRITE_CMD	5
#define EE_READ_CMD	6
#define EE_ERASE_CMD	7

static void rtl8139_eeprom_delay(struct rtl8139_priv *priv)
{
	/*
	 * Delay between EEPROM clock transitions.
	 * No extra delay is needed with 33MHz PCI, but 66MHz may change this.
	 */
	inl(priv->ioaddr + RTL_REG_CFG9346);
}

static int rtl8139_read_eeprom(struct rtl8139_priv *priv,
			       unsigned int location, unsigned int addr_len)
{
	unsigned int read_cmd = location | (EE_READ_CMD << addr_len);
	uintptr_t ee_addr = priv->ioaddr + RTL_REG_CFG9346;
	unsigned int retval = 0;
	u8 dataval;
	int i;

	outb(EE_ENB & ~EE_CS, ee_addr);
	outb(EE_ENB, ee_addr);
	rtl8139_eeprom_delay(priv);

	/* Shift the read command bits out. */
	for (i = 4 + addr_len; i >= 0; i--) {
		dataval = (read_cmd & BIT(i)) ? EE_DATA_WRITE : 0;
		outb(EE_ENB | dataval, ee_addr);
		rtl8139_eeprom_delay(priv);
		outb(EE_ENB | dataval | EE_SHIFT_CLK, ee_addr);
		rtl8139_eeprom_delay(priv);
	}

	outb(EE_ENB, ee_addr);
	rtl8139_eeprom_delay(priv);

	for (i = 16; i > 0; i--) {
		outb(EE_ENB | EE_SHIFT_CLK, ee_addr);
		rtl8139_eeprom_delay(priv);
		retval <<= 1;
		retval |= inb(ee_addr) & EE_DATA_READ;
		outb(EE_ENB, ee_addr);
		rtl8139_eeprom_delay(priv);
	}

	/* Terminate the EEPROM access. */
	outb(~EE_CS, ee_addr);
	rtl8139_eeprom_delay(priv);

	return retval;
}

static const unsigned int rtl8139_rx_config =
	(RX_BUF_LEN_IDX << 11) |
	(RX_FIFO_THRESH << 13) |
	(RX_DMA_BURST << 8);

static void rtl8139_set_rx_mode(struct rtl8139_priv *priv)
{
	/* !IFF_PROMISC */
	unsigned int rx_mode = RTL_REG_RXCONFIG_ACCEPTBROADCAST |
			       RTL_REG_RXCONFIG_ACCEPTMULTICAST |
			       RTL_REG_RXCONFIG_ACCEPTMYPHYS;

	outl(rtl8139_rx_config | rx_mode, priv->ioaddr + RTL_REG_RXCONFIG);

	outl(0xffffffff, priv->ioaddr + RTL_REG_MAR0 + 0);
	outl(0xffffffff, priv->ioaddr + RTL_REG_MAR0 + 4);
}

static void rtl8139_hw_reset(struct rtl8139_priv *priv)
{
	u8 reg;
	int i;

	outb(RTL_REG_CHIPCMD_CMDRESET, priv->ioaddr + RTL_REG_CHIPCMD);

	/* Give the chip 10ms to finish the reset. */
	for (i = 0; i < 100; i++) {
		reg = inb(priv->ioaddr + RTL_REG_CHIPCMD);
		if (!(reg & RTL_REG_CHIPCMD_CMDRESET))
			break;

		udelay(100);
	}
}

static void rtl8139_reset(struct rtl8139_priv *priv)
{
	int i;

	priv->cur_rx = 0;
	priv->cur_tx = 0;

	rtl8139_hw_reset(priv);

	for (i = 0; i < ETH_ALEN; i++)
		outb(priv->enetaddr[i], priv->ioaddr + RTL_REG_MAC0 + i);

	/* Must enable Tx/Rx before setting transfer thresholds! */
	outb(RTL_REG_CHIPCMD_CMDRXENB | RTL_REG_CHIPCMD_CMDTXENB,
	     priv->ioaddr + RTL_REG_CHIPCMD);

	/* accept no frames yet! */
	outl(rtl8139_rx_config, priv->ioaddr + RTL_REG_RXCONFIG);
	outl((TX_DMA_BURST << 8) | 0x03000000, priv->ioaddr + RTL_REG_TXCONFIG);

	/*
	 * The Linux driver changes RTL_REG_CONFIG1 here to use a different
	 * LED pattern for half duplex or full/autodetect duplex (for
	 * full/autodetect, the outputs are TX/RX, Link10/100, FULL, while
	 * for half duplex it uses TX/RX, Link100, Link10).  This is messy,
	 * because it doesn't match the inscription on the mounting bracket.
	 * It should not be changed from the configuration EEPROM default,
	 * because the card manufacturer should have set that to match the
	 * card.
	 */
	debug_cond(DEBUG_RX, "rx ring address is %p\n", rx_ring);

	flush_cache((unsigned long)rx_ring, RX_BUF_LEN);
	outl(phys_to_bus(priv->devno, (int)rx_ring), priv->ioaddr + RTL_REG_RXBUF);

	/*
	 * If we add multicast support, the RTL_REG_MAR0 register would have
	 * to be initialized to 0xffffffffffffffff (two 32 bit accesses).
	 * Etherboot only needs broadcast (for ARP/RARP/BOOTP/DHCP) and
	 * unicast.
	 */
	outb(RTL_REG_CHIPCMD_CMDRXENB | RTL_REG_CHIPCMD_CMDTXENB,
	     priv->ioaddr + RTL_REG_CHIPCMD);

	outl(rtl8139_rx_config, priv->ioaddr + RTL_REG_RXCONFIG);

	/* Start the chip's Tx and Rx process. */
	outl(0, priv->ioaddr + RTL_REG_RXMISSED);

	rtl8139_set_rx_mode(priv);

	/* Disable all known interrupts by setting the interrupt mask. */
	outw(0, priv->ioaddr + RTL_REG_INTRMASK);
}

static int rtl8139_send_common(struct rtl8139_priv *priv,
			       void *packet, int length)
{
	unsigned int len = length;
	unsigned long txstatus;
	unsigned int status;
	int i = 0;

	memcpy(tx_buffer, packet, length);

	debug_cond(DEBUG_TX, "sending %d bytes\n", len);

	/*
	 * Note: RTL8139 doesn't auto-pad, send minimum payload (another 4
	 * bytes are sent automatically for the FCS, totalling to 64 bytes).
	 */
	while (len < ETH_ZLEN)
		tx_buffer[len++] = '\0';

	flush_cache((unsigned long)tx_buffer, length);
	outl(phys_to_bus(priv->devno, (unsigned long)tx_buffer),
	     priv->ioaddr + RTL_REG_TXADDR0 + priv->cur_tx * 4);
	outl(((TX_FIFO_THRESH << 11) & 0x003f0000) | len,
	     priv->ioaddr + RTL_REG_TXSTATUS0 + priv->cur_tx * 4);

	do {
		status = inw(priv->ioaddr + RTL_REG_INTRSTATUS);
		/*
		 * Only acknlowledge interrupt sources we can properly
		 * handle here - the RTL_REG_INTRSTATUS_RXOVERFLOW/
		 * RTL_REG_INTRSTATUS_RXFIFOOVER MUST be handled in the
		 * rtl8139_recv() function.
		 */
		status &= RTL_REG_INTRSTATUS_TXOK | RTL_REG_INTRSTATUS_TXERR |
			  RTL_REG_INTRSTATUS_PCIERR;
		outw(status, priv->ioaddr + RTL_REG_INTRSTATUS);
		if (status)
			break;

		udelay(10);
	} while (i++ < RTL_TIMEOUT);

	txstatus = inl(priv->ioaddr + RTL_REG_TXSTATUS0 + priv->cur_tx * 4);

	if (!(status & RTL_REG_INTRSTATUS_TXOK)) {
		debug_cond(DEBUG_TX,
			   "tx timeout/error (%d usecs), status %hX txstatus %lX\n",
			   10 * i, status, txstatus);

		rtl8139_reset(priv);

		return 0;
	}

	priv->cur_tx = (priv->cur_tx + 1) % NUM_TX_DESC;

	debug_cond(DEBUG_TX, "tx done, status %hX txstatus %lX\n",
		   status, txstatus);

	return length;
}

static int rtl8139_recv_common(struct rtl8139_priv *priv, unsigned char *rxdata,
			       uchar **packetp)
{
	const unsigned int rxstat = RTL_REG_INTRSTATUS_RXFIFOOVER |
				    RTL_REG_INTRSTATUS_RXOVERFLOW |
				    RTL_REG_INTRSTATUS_RXOK;
	unsigned int rx_size, rx_status;
	unsigned int ring_offs;
	int length = 0;

	if (inb(priv->ioaddr + RTL_REG_CHIPCMD) & RTL_REG_CHIPCMD_RXBUFEMPTY)
		return 0;

	priv->rxstatus = inw(priv->ioaddr + RTL_REG_INTRSTATUS);
	/* See below for the rest of the interrupt acknowledges.  */
	outw(priv->rxstatus & ~rxstat, priv->ioaddr + RTL_REG_INTRSTATUS);

	debug_cond(DEBUG_RX, "%s: int %hX ", __func__, priv->rxstatus);

	ring_offs = priv->cur_rx % RX_BUF_LEN;
	/* ring_offs is guaranteed being 4-byte aligned */
	rx_status = le32_to_cpu(*(unsigned int *)(rx_ring + ring_offs));
	rx_size = rx_status >> 16;
	rx_status &= 0xffff;

	if ((rx_status & (RTL_STS_RXBADSYMBOL | RTL_STS_RXRUNT |
			  RTL_STS_RXTOOLONG | RTL_STS_RXCRCERR |
			  RTL_STS_RXBADALIGN)) ||
	    (rx_size < ETH_ZLEN) ||
	    (rx_size > ETH_FRAME_LEN + 4)) {
		debug("rx error %hX\n", rx_status);
		/* this clears all interrupts still pending */
		rtl8139_reset(priv);
		return 0;
	}

	/* Received a good packet */
	length = rx_size - 4;	/* no one cares about the FCS */
	if (ring_offs + 4 + rx_size - 4 > RX_BUF_LEN) {
		int semi_count = RX_BUF_LEN - ring_offs - 4;

		memcpy(rxdata, rx_ring + ring_offs + 4, semi_count);
		memcpy(&rxdata[semi_count], rx_ring,
		       rx_size - 4 - semi_count);

		*packetp = rxdata;
		debug_cond(DEBUG_RX, "rx packet %d+%d bytes",
			   semi_count, rx_size - 4 - semi_count);
	} else {
		*packetp = rx_ring + ring_offs + 4;
		debug_cond(DEBUG_RX, "rx packet %d bytes", rx_size - 4);
	}

	return length;
}

static int rtl8139_free_pkt_common(struct rtl8139_priv *priv, unsigned int len)
{
	const unsigned int rxstat = RTL_REG_INTRSTATUS_RXFIFOOVER |
				    RTL_REG_INTRSTATUS_RXOVERFLOW |
				    RTL_REG_INTRSTATUS_RXOK;
	unsigned int rx_size = len + 4;

	flush_cache((unsigned long)rx_ring, RX_BUF_LEN);

	priv->cur_rx = ROUND(priv->cur_rx + rx_size + 4, 4);
	outw(priv->cur_rx - 16, priv->ioaddr + RTL_REG_RXBUFPTR);
	/*
	 * See RTL8139 Programming Guide V0.1 for the official handling of
	 * Rx overflow situations. The document itself contains basically
	 * no usable information, except for a few exception handling rules.
	 */
	outw(priv->rxstatus & rxstat, priv->ioaddr + RTL_REG_INTRSTATUS);

	return 0;
}

static int rtl8139_init_common(struct rtl8139_priv *priv)
{
	u8 reg;

	/* Bring the chip out of low-power mode. */
	outb(0x00, priv->ioaddr + RTL_REG_CONFIG1);

	rtl8139_reset(priv);

	reg = inb(priv->ioaddr + RTL_REG_MEDIASTATUS);
	if (reg & RTL_REG_MEDIASTATUS_MSRLINKFAIL) {
		printf("Cable not connected or other link failure\n");
		return -1;
	}

	return 0;
}

static void rtl8139_stop_common(struct rtl8139_priv *priv)
{
	rtl8139_hw_reset(priv);
}

static void rtl8139_get_hwaddr(struct rtl8139_priv *priv)
{
	unsigned short *ap = (unsigned short *)priv->enetaddr;
	int i, addr_len;

	/* Bring the chip out of low-power mode. */
	outb(0x00, priv->ioaddr + RTL_REG_CONFIG1);

	addr_len = rtl8139_read_eeprom(priv, 0, 8) == 0x8129 ? 8 : 6;
	for (i = 0; i < 3; i++)
		*ap++ = le16_to_cpu(rtl8139_read_eeprom(priv, i + 7, addr_len));
}

static void rtl8139_name(char *str, int card_number)
{
	sprintf(str, "RTL8139#%u", card_number);
}

static struct pci_device_id supported[] = {
	{ PCI_DEVICE(PCI_VENDOR_ID_REALTEK, PCI_DEVICE_ID_REALTEK_8139) },
	{ PCI_DEVICE(PCI_VENDOR_ID_DLINK, PCI_DEVICE_ID_DLINK_8139) },
	{ }
};

static int rtl8139_start(struct udevice *dev)
{
	struct eth_pdata *plat = dev_get_plat(dev);
	struct rtl8139_priv *priv = dev_get_priv(dev);

	memcpy(priv->enetaddr, plat->enetaddr, sizeof(plat->enetaddr));

	return rtl8139_init_common(priv);
}

static void rtl8139_stop(struct udevice *dev)
{
	struct rtl8139_priv *priv = dev_get_priv(dev);

	rtl8139_stop_common(priv);
}

static int rtl8139_send(struct udevice *dev, void *packet, int length)
{
	struct rtl8139_priv *priv = dev_get_priv(dev);
	int ret;

	ret = rtl8139_send_common(priv, packet, length);

	return ret ? 0 : -ETIMEDOUT;
}

static int rtl8139_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct rtl8139_priv *priv = dev_get_priv(dev);
	static unsigned char rxdata[RX_BUF_LEN];

	return rtl8139_recv_common(priv, rxdata, packetp);
}

static int rtl8139_free_pkt(struct udevice *dev, uchar *packet, int length)
{
	struct rtl8139_priv *priv = dev_get_priv(dev);

	rtl8139_free_pkt_common(priv, length);

	return 0;
}

static int rtl8139_write_hwaddr(struct udevice *dev)
{
	struct eth_pdata *plat = dev_get_plat(dev);
	struct rtl8139_priv *priv = dev_get_priv(dev);

	memcpy(priv->enetaddr, plat->enetaddr, sizeof(plat->enetaddr));

	rtl8139_reset(priv);

	return 0;
}

static int rtl8139_read_rom_hwaddr(struct udevice *dev)
{
	struct rtl8139_priv *priv = dev_get_priv(dev);

	rtl8139_get_hwaddr(priv);

	return 0;
}

static int rtl8139_bind(struct udevice *dev)
{
	static int card_number;
	char name[16];

	rtl8139_name(name, card_number++);

	return device_set_name(dev, name);
}

static int rtl8139_probe(struct udevice *dev)
{
	struct eth_pdata *plat = dev_get_plat(dev);
	struct rtl8139_priv *priv = dev_get_priv(dev);
	u32 iobase;

	dm_pci_read_config32(dev, PCI_BASE_ADDRESS_1, &iobase);
	iobase &= ~0xf;

	debug("rtl8139: REALTEK RTL8139 @0x%x\n", iobase);

	priv->devno = dev;
	priv->ioaddr = (unsigned long)bus_to_phys(dev, iobase);

	rtl8139_get_hwaddr(priv);
	memcpy(plat->enetaddr, priv->enetaddr, sizeof(priv->enetaddr));

	dm_pci_write_config8(dev, PCI_LATENCY_TIMER, 0x20);

	return 0;
}

static const struct eth_ops rtl8139_ops = {
	.start		= rtl8139_start,
	.send		= rtl8139_send,
	.recv		= rtl8139_recv,
	.stop		= rtl8139_stop,
	.free_pkt	= rtl8139_free_pkt,
	.write_hwaddr	= rtl8139_write_hwaddr,
	.read_rom_hwaddr = rtl8139_read_rom_hwaddr,
};

U_BOOT_DRIVER(eth_rtl8139) = {
	.name	= "eth_rtl8139",
	.id	= UCLASS_ETH,
	.bind	= rtl8139_bind,
	.probe	= rtl8139_probe,
	.ops	= &rtl8139_ops,
	.priv_auto	= sizeof(struct rtl8139_priv),
	.plat_auto	= sizeof(struct eth_pdata),
};

U_BOOT_PCI_DEVICE(eth_rtl8139, supported);
