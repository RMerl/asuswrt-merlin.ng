// SPDX-License-Identifier: GPL-2.0+
/*
 * rtl8169.c : U-Boot driver for the RealTek RTL8169
 *
 * Masami Komiya (mkomiya@sonare.it)
 *
 * Most part is taken from r8169.c of etherboot
 *
 */

/**************************************************************************
*    r8169.c: Etherboot device driver for the RealTek RTL-8169 Gigabit
*    Written 2003 by Timothy Legge <tlegge@rogers.com>
*
*    Portions of this code based on:
*	r8169.c: A RealTek RTL-8169 Gigabit Ethernet driver
*		for Linux kernel 2.4.x.
*
*    Written 2002 ShuChen <shuchen@realtek.com.tw>
*	  See Linux Driver for full information
*
*    Linux Driver Version 1.27a, 10.02.2002
*
*    Thanks to:
*	Jean Chen of RealTek Semiconductor Corp. for
*	providing the evaluation NIC used to develop
*	this driver.  RealTek's support for Etherboot
*	is appreciated.
*
*    REVISION HISTORY:
*    ================
*
*    v1.0	11-26-2003	timlegge	Initial port of Linux driver
*    v1.5	01-17-2004	timlegge	Initial driver output cleanup
*
*    Indent Options: indent -kr -i8
***************************************************************************/
/*
 * 26 August 2006 Mihai Georgian <u-boot@linuxnotincluded.org.uk>
 * Modified to use le32_to_cpu and cpu_to_le32 properly
 */
#include <common.h>
#include <dm.h>
#include <errno.h>
#include <malloc.h>
#include <memalign.h>
#include <net.h>
#ifndef CONFIG_DM_ETH
#include <netdev.h>
#endif
#include <asm/io.h>
#include <pci.h>

#undef DEBUG_RTL8169
#undef DEBUG_RTL8169_TX
#undef DEBUG_RTL8169_RX

#define drv_version "v1.5"
#define drv_date "01-17-2004"

static unsigned long ioaddr;

/* Condensed operations for readability. */
#define currticks()	get_timer(0)

/* media options */
#define MAX_UNITS 8
static int media[MAX_UNITS] = { -1, -1, -1, -1, -1, -1, -1, -1 };

/* MAC address length*/
#define MAC_ADDR_LEN	6

/* max supported gigabit ethernet frame size -- must be at least (dev->mtu+14+4).*/
#define MAX_ETH_FRAME_SIZE	1536

#define TX_FIFO_THRESH 256	/* In bytes */

#define RX_FIFO_THRESH	7	/* 7 means NO threshold, Rx buffer level before first PCI xfer.	 */
#define RX_DMA_BURST	6	/* Maximum PCI burst, '6' is 1024 */
#define TX_DMA_BURST	6	/* Maximum PCI burst, '6' is 1024 */
#define EarlyTxThld	0x3F	/* 0x3F means NO early transmit */
#define RxPacketMaxSize 0x0800	/* Maximum size supported is 16K-1 */
#define InterFrameGap	0x03	/* 3 means InterFrameGap = the shortest one */

#define NUM_TX_DESC	1	/* Number of Tx descriptor registers */
#ifdef CONFIG_SYS_RX_ETH_BUFFER
  #define NUM_RX_DESC	CONFIG_SYS_RX_ETH_BUFFER
#else
  #define NUM_RX_DESC	4	/* Number of Rx descriptor registers */
#endif
#define RX_BUF_SIZE	1536	/* Rx Buffer size */
#define RX_BUF_LEN	8192

#define RTL_MIN_IO_SIZE 0x80
#define TX_TIMEOUT  (6*HZ)

/* write/read MMIO register. Notice: {read,write}[wl] do the necessary swapping */
#define RTL_W8(reg, val8)	writeb((val8), ioaddr + (reg))
#define RTL_W16(reg, val16)	writew((val16), ioaddr + (reg))
#define RTL_W32(reg, val32)	writel((val32), ioaddr + (reg))
#define RTL_R8(reg)		readb(ioaddr + (reg))
#define RTL_R16(reg)		readw(ioaddr + (reg))
#define RTL_R32(reg)		readl(ioaddr + (reg))

#define bus_to_phys(a)	pci_mem_to_phys((pci_dev_t)(unsigned long)dev->priv, \
	(pci_addr_t)(unsigned long)a)
#define phys_to_bus(a)	pci_phys_to_mem((pci_dev_t)(unsigned long)dev->priv, \
	(phys_addr_t)a)

enum RTL8169_registers {
	MAC0 = 0,		/* Ethernet hardware address. */
	MAR0 = 8,		/* Multicast filter. */
	TxDescStartAddrLow = 0x20,
	TxDescStartAddrHigh = 0x24,
	TxHDescStartAddrLow = 0x28,
	TxHDescStartAddrHigh = 0x2c,
	FLASH = 0x30,
	ERSR = 0x36,
	ChipCmd = 0x37,
	TxPoll = 0x38,
	IntrMask = 0x3C,
	IntrStatus = 0x3E,
	TxConfig = 0x40,
	RxConfig = 0x44,
	RxMissed = 0x4C,
	Cfg9346 = 0x50,
	Config0 = 0x51,
	Config1 = 0x52,
	Config2 = 0x53,
	Config3 = 0x54,
	Config4 = 0x55,
	Config5 = 0x56,
	MultiIntr = 0x5C,
	PHYAR = 0x60,
	TBICSR = 0x64,
	TBI_ANAR = 0x68,
	TBI_LPAR = 0x6A,
	PHYstatus = 0x6C,
	RxMaxSize = 0xDA,
	CPlusCmd = 0xE0,
	RxDescStartAddrLow = 0xE4,
	RxDescStartAddrHigh = 0xE8,
	EarlyTxThres = 0xEC,
	FuncEvent = 0xF0,
	FuncEventMask = 0xF4,
	FuncPresetState = 0xF8,
	FuncForceEvent = 0xFC,
};

enum RTL8169_register_content {
	/*InterruptStatusBits */
	SYSErr = 0x8000,
	PCSTimeout = 0x4000,
	SWInt = 0x0100,
	TxDescUnavail = 0x80,
	RxFIFOOver = 0x40,
	RxUnderrun = 0x20,
	RxOverflow = 0x10,
	TxErr = 0x08,
	TxOK = 0x04,
	RxErr = 0x02,
	RxOK = 0x01,

	/*RxStatusDesc */
	RxRES = 0x00200000,
	RxCRC = 0x00080000,
	RxRUNT = 0x00100000,
	RxRWT = 0x00400000,

	/*ChipCmdBits */
	CmdReset = 0x10,
	CmdRxEnb = 0x08,
	CmdTxEnb = 0x04,
	RxBufEmpty = 0x01,

	/*Cfg9346Bits */
	Cfg9346_Lock = 0x00,
	Cfg9346_Unlock = 0xC0,

	/*rx_mode_bits */
	AcceptErr = 0x20,
	AcceptRunt = 0x10,
	AcceptBroadcast = 0x08,
	AcceptMulticast = 0x04,
	AcceptMyPhys = 0x02,
	AcceptAllPhys = 0x01,

	/*RxConfigBits */
	RxCfgFIFOShift = 13,
	RxCfgDMAShift = 8,

	/*TxConfigBits */
	TxInterFrameGapShift = 24,
	TxDMAShift = 8,		/* DMA burst value (0-7) is shift this many bits */

	/*rtl8169_PHYstatus */
	TBI_Enable = 0x80,
	TxFlowCtrl = 0x40,
	RxFlowCtrl = 0x20,
	_1000bpsF = 0x10,
	_100bps = 0x08,
	_10bps = 0x04,
	LinkStatus = 0x02,
	FullDup = 0x01,

	/*GIGABIT_PHY_registers */
	PHY_CTRL_REG = 0,
	PHY_STAT_REG = 1,
	PHY_AUTO_NEGO_REG = 4,
	PHY_1000_CTRL_REG = 9,

	/*GIGABIT_PHY_REG_BIT */
	PHY_Restart_Auto_Nego = 0x0200,
	PHY_Enable_Auto_Nego = 0x1000,

	/* PHY_STAT_REG = 1; */
	PHY_Auto_Nego_Comp = 0x0020,

	/* PHY_AUTO_NEGO_REG = 4; */
	PHY_Cap_10_Half = 0x0020,
	PHY_Cap_10_Full = 0x0040,
	PHY_Cap_100_Half = 0x0080,
	PHY_Cap_100_Full = 0x0100,

	/* PHY_1000_CTRL_REG = 9; */
	PHY_Cap_1000_Full = 0x0200,

	PHY_Cap_Null = 0x0,

	/*_MediaType*/
	_10_Half = 0x01,
	_10_Full = 0x02,
	_100_Half = 0x04,
	_100_Full = 0x08,
	_1000_Full = 0x10,

	/*_TBICSRBit*/
	TBILinkOK = 0x02000000,
};

static struct {
	const char *name;
	u8 version;		/* depend on RTL8169 docs */
	u32 RxConfigMask;	/* should clear the bits supported by this chip */
} rtl_chip_info[] = {
	{"RTL-8169", 0x00, 0xff7e1880,},
	{"RTL-8169", 0x04, 0xff7e1880,},
	{"RTL-8169", 0x00, 0xff7e1880,},
	{"RTL-8169s/8110s",	0x02, 0xff7e1880,},
	{"RTL-8169s/8110s",	0x04, 0xff7e1880,},
	{"RTL-8169sb/8110sb",	0x10, 0xff7e1880,},
	{"RTL-8169sc/8110sc",	0x18, 0xff7e1880,},
	{"RTL-8168b/8111sb",	0x30, 0xff7e1880,},
	{"RTL-8168b/8111sb",	0x38, 0xff7e1880,},
	{"RTL-8168d/8111d",	0x28, 0xff7e1880,},
	{"RTL-8168evl/8111evl",	0x2e, 0xff7e1880,},
	{"RTL-8168/8111g",	0x4c, 0xff7e1880,},
	{"RTL-8101e",		0x34, 0xff7e1880,},
	{"RTL-8100e",		0x32, 0xff7e1880,},
	{"RTL-8168h/8111h",	0x54, 0xff7e1880,},
};

enum _DescStatusBit {
	OWNbit = 0x80000000,
	EORbit = 0x40000000,
	FSbit = 0x20000000,
	LSbit = 0x10000000,
};

struct TxDesc {
	u32 status;
	u32 vlan_tag;
	u32 buf_addr;
	u32 buf_Haddr;
};

struct RxDesc {
	u32 status;
	u32 vlan_tag;
	u32 buf_addr;
	u32 buf_Haddr;
};

static unsigned char rxdata[RX_BUF_LEN];

#define RTL8169_DESC_SIZE 16

#if ARCH_DMA_MINALIGN > 256
#  define RTL8169_ALIGN ARCH_DMA_MINALIGN
#else
#  define RTL8169_ALIGN 256
#endif

/*
 * Warn if the cache-line size is larger than the descriptor size. In such
 * cases the driver will likely fail because the CPU needs to flush the cache
 * when requeuing RX buffers, therefore descriptors written by the hardware
 * may be discarded.
 *
 * This can be fixed by defining CONFIG_SYS_NONCACHED_MEMORY which will cause
 * the driver to allocate descriptors from a pool of non-cached memory.
 */
#if RTL8169_DESC_SIZE < ARCH_DMA_MINALIGN
#if !defined(CONFIG_SYS_NONCACHED_MEMORY) && \
	!CONFIG_IS_ENABLED(SYS_DCACHE_OFF) && !defined(CONFIG_X86)
#warning cache-line size is larger than descriptor size
#endif
#endif

/*
 * Create a static buffer of size RX_BUF_SZ for each TX Descriptor. All
 * descriptors point to a part of this buffer.
 */
DEFINE_ALIGN_BUFFER(u8, txb, NUM_TX_DESC * RX_BUF_SIZE, RTL8169_ALIGN);

/*
 * Create a static buffer of size RX_BUF_SZ for each RX Descriptor. All
 * descriptors point to a part of this buffer.
 */
DEFINE_ALIGN_BUFFER(u8, rxb, NUM_RX_DESC * RX_BUF_SIZE, RTL8169_ALIGN);

struct rtl8169_private {
	ulong iobase;
	void *mmio_addr;	/* memory map physical address */
	int chipset;
	unsigned long cur_rx;	/* Index into the Rx descriptor buffer of next Rx pkt. */
	unsigned long cur_tx;	/* Index into the Tx descriptor buffer of next Rx pkt. */
	unsigned long dirty_tx;
	struct TxDesc *TxDescArray;	/* Index of 256-alignment Tx Descriptor buffer */
	struct RxDesc *RxDescArray;	/* Index of 256-alignment Rx Descriptor buffer */
	unsigned char *RxBufferRings;	/* Index of Rx Buffer  */
	unsigned char *RxBufferRing[NUM_RX_DESC];	/* Index of Rx Buffer array */
	unsigned char *Tx_skbuff[NUM_TX_DESC];
} tpx;

static struct rtl8169_private *tpc;

static const unsigned int rtl8169_rx_config =
    (RX_FIFO_THRESH << RxCfgFIFOShift) | (RX_DMA_BURST << RxCfgDMAShift);

static struct pci_device_id supported[] = {
	{ PCI_DEVICE(PCI_VENDOR_ID_REALTEK, 0x8167) },
	{ PCI_DEVICE(PCI_VENDOR_ID_REALTEK, 0x8168) },
	{ PCI_DEVICE(PCI_VENDOR_ID_REALTEK, 0x8169) },
	{}
};

void mdio_write(int RegAddr, int value)
{
	int i;

	RTL_W32(PHYAR, 0x80000000 | (RegAddr & 0xFF) << 16 | value);
	udelay(1000);

	for (i = 2000; i > 0; i--) {
		/* Check if the RTL8169 has completed writing to the specified MII register */
		if (!(RTL_R32(PHYAR) & 0x80000000)) {
			break;
		} else {
			udelay(100);
		}
	}
}

int mdio_read(int RegAddr)
{
	int i, value = -1;

	RTL_W32(PHYAR, 0x0 | (RegAddr & 0xFF) << 16);
	udelay(1000);

	for (i = 2000; i > 0; i--) {
		/* Check if the RTL8169 has completed retrieving data from the specified MII register */
		if (RTL_R32(PHYAR) & 0x80000000) {
			value = (int) (RTL_R32(PHYAR) & 0xFFFF);
			break;
		} else {
			udelay(100);
		}
	}
	return value;
}

static int rtl8169_init_board(unsigned long dev_iobase, const char *name)
{
	int i;
	u32 tmp;

#ifdef DEBUG_RTL8169
	printf ("%s\n", __FUNCTION__);
#endif
	ioaddr = dev_iobase;

	/* Soft reset the chip. */
	RTL_W8(ChipCmd, CmdReset);

	/* Check that the chip has finished the reset. */
	for (i = 1000; i > 0; i--)
		if ((RTL_R8(ChipCmd) & CmdReset) == 0)
			break;
		else
			udelay(10);

	/* identify chip attached to board */
	tmp = RTL_R32(TxConfig);
	tmp = ((tmp & 0x7c000000) + ((tmp & 0x00800000) << 2)) >> 24;

	for (i = ARRAY_SIZE(rtl_chip_info) - 1; i >= 0; i--){
		if (tmp == rtl_chip_info[i].version) {
			tpc->chipset = i;
			goto match;
		}
	}

	/* if unknown chip, assume array element #0, original RTL-8169 in this case */
	printf("PCI device %s: unknown chip version, assuming RTL-8169\n",
	       name);
	printf("PCI device: TxConfig = 0x%lX\n", (unsigned long) RTL_R32(TxConfig));
	tpc->chipset = 0;

match:
	return 0;
}

/*
 * TX and RX descriptors are 16 bytes. This causes problems with the cache
 * maintenance on CPUs where the cache-line size exceeds the size of these
 * descriptors. What will happen is that when the driver receives a packet
 * it will be immediately requeued for the hardware to reuse. The CPU will
 * therefore need to flush the cache-line containing the descriptor, which
 * will cause all other descriptors in the same cache-line to be flushed
 * along with it. If one of those descriptors had been written to by the
 * device those changes (and the associated packet) will be lost.
 *
 * To work around this, we make use of non-cached memory if available. If
 * descriptors are mapped uncached there's no need to manually flush them
 * or invalidate them.
 *
 * Note that this only applies to descriptors. The packet data buffers do
 * not have the same constraints since they are 1536 bytes large, so they
 * are unlikely to share cache-lines.
 */
static void *rtl_alloc_descs(unsigned int num)
{
	size_t size = num * RTL8169_DESC_SIZE;

#ifdef CONFIG_SYS_NONCACHED_MEMORY
	return (void *)noncached_alloc(size, RTL8169_ALIGN);
#else
	return memalign(RTL8169_ALIGN, size);
#endif
}

/*
 * Cache maintenance functions. These are simple wrappers around the more
 * general purpose flush_cache() and invalidate_dcache_range() functions.
 */

static void rtl_inval_rx_desc(struct RxDesc *desc)
{
#ifndef CONFIG_SYS_NONCACHED_MEMORY
	unsigned long start = (unsigned long)desc & ~(ARCH_DMA_MINALIGN - 1);
	unsigned long end = ALIGN(start + sizeof(*desc), ARCH_DMA_MINALIGN);

	invalidate_dcache_range(start, end);
#endif
}

static void rtl_flush_rx_desc(struct RxDesc *desc)
{
#ifndef CONFIG_SYS_NONCACHED_MEMORY
	flush_cache((unsigned long)desc, sizeof(*desc));
#endif
}

static void rtl_inval_tx_desc(struct TxDesc *desc)
{
#ifndef CONFIG_SYS_NONCACHED_MEMORY
	unsigned long start = (unsigned long)desc & ~(ARCH_DMA_MINALIGN - 1);
	unsigned long end = ALIGN(start + sizeof(*desc), ARCH_DMA_MINALIGN);

	invalidate_dcache_range(start, end);
#endif
}

static void rtl_flush_tx_desc(struct TxDesc *desc)
{
#ifndef CONFIG_SYS_NONCACHED_MEMORY
	flush_cache((unsigned long)desc, sizeof(*desc));
#endif
}

static void rtl_inval_buffer(void *buf, size_t size)
{
	unsigned long start = (unsigned long)buf & ~(ARCH_DMA_MINALIGN - 1);
	unsigned long end = ALIGN(start + size, ARCH_DMA_MINALIGN);

	invalidate_dcache_range(start, end);
}

static void rtl_flush_buffer(void *buf, size_t size)
{
	flush_cache((unsigned long)buf, size);
}

/**************************************************************************
RECV - Receive a frame
***************************************************************************/
#ifdef CONFIG_DM_ETH
static int rtl_recv_common(struct udevice *dev, unsigned long dev_iobase,
			   uchar **packetp)
#else
static int rtl_recv_common(pci_dev_t dev, unsigned long dev_iobase,
			   uchar **packetp)
#endif
{
	/* return true if there's an ethernet packet ready to read */
	/* nic->packet should contain data on return */
	/* nic->packetlen should contain length of data */
	int cur_rx;
	int length = 0;

#ifdef DEBUG_RTL8169_RX
	printf ("%s\n", __FUNCTION__);
#endif
	ioaddr = dev_iobase;

	cur_rx = tpc->cur_rx;

	rtl_inval_rx_desc(&tpc->RxDescArray[cur_rx]);

	if ((le32_to_cpu(tpc->RxDescArray[cur_rx].status) & OWNbit) == 0) {
		if (!(le32_to_cpu(tpc->RxDescArray[cur_rx].status) & RxRES)) {
			length = (int) (le32_to_cpu(tpc->RxDescArray[cur_rx].
						status) & 0x00001FFF) - 4;

			rtl_inval_buffer(tpc->RxBufferRing[cur_rx], length);
			memcpy(rxdata, tpc->RxBufferRing[cur_rx], length);

			if (cur_rx == NUM_RX_DESC - 1)
				tpc->RxDescArray[cur_rx].status =
					cpu_to_le32((OWNbit | EORbit) + RX_BUF_SIZE);
			else
				tpc->RxDescArray[cur_rx].status =
					cpu_to_le32(OWNbit + RX_BUF_SIZE);
#ifdef CONFIG_DM_ETH
			tpc->RxDescArray[cur_rx].buf_addr = cpu_to_le32(
				dm_pci_mem_to_phys(dev,
					(pci_addr_t)(unsigned long)
					tpc->RxBufferRing[cur_rx]));
#else
			tpc->RxDescArray[cur_rx].buf_addr = cpu_to_le32(
				pci_mem_to_phys(dev, (pci_addr_t)(unsigned long)
				tpc->RxBufferRing[cur_rx]));
#endif
			rtl_flush_rx_desc(&tpc->RxDescArray[cur_rx]);
#ifdef CONFIG_DM_ETH
			*packetp = rxdata;
#else
			net_process_received_packet(rxdata, length);
#endif
		} else {
			puts("Error Rx");
			length = -EIO;
		}
		cur_rx = (cur_rx + 1) % NUM_RX_DESC;
		tpc->cur_rx = cur_rx;
		return length;

	} else {
		ushort sts = RTL_R8(IntrStatus);
		RTL_W8(IntrStatus, sts & ~(TxErr | RxErr | SYSErr));
		udelay(100);	/* wait */
	}
	tpc->cur_rx = cur_rx;
	return (0);		/* initially as this is called to flush the input */
}

#ifdef CONFIG_DM_ETH
int rtl8169_eth_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct rtl8169_private *priv = dev_get_priv(dev);

	return rtl_recv_common(dev, priv->iobase, packetp);
}
#else
static int rtl_recv(struct eth_device *dev)
{
	return rtl_recv_common((pci_dev_t)(unsigned long)dev->priv,
			       dev->iobase, NULL);
}
#endif /* nCONFIG_DM_ETH */

#define HZ 1000
/**************************************************************************
SEND - Transmit a frame
***************************************************************************/
#ifdef CONFIG_DM_ETH
static int rtl_send_common(struct udevice *dev, unsigned long dev_iobase,
			   void *packet, int length)
#else
static int rtl_send_common(pci_dev_t dev, unsigned long dev_iobase,
			   void *packet, int length)
#endif
{
	/* send the packet to destination */

	u32 to;
	u8 *ptxb;
	int entry = tpc->cur_tx % NUM_TX_DESC;
	u32 len = length;
	int ret;

#ifdef DEBUG_RTL8169_TX
	int stime = currticks();
	printf ("%s\n", __FUNCTION__);
	printf("sending %d bytes\n", len);
#endif

	ioaddr = dev_iobase;

	/* point to the current txb incase multiple tx_rings are used */
	ptxb = tpc->Tx_skbuff[entry * MAX_ETH_FRAME_SIZE];
	memcpy(ptxb, (char *)packet, (int)length);

	while (len < ETH_ZLEN)
		ptxb[len++] = '\0';

	rtl_flush_buffer(ptxb, ALIGN(len, RTL8169_ALIGN));

	tpc->TxDescArray[entry].buf_Haddr = 0;
#ifdef CONFIG_DM_ETH
	tpc->TxDescArray[entry].buf_addr = cpu_to_le32(
		dm_pci_mem_to_phys(dev, (pci_addr_t)(unsigned long)ptxb));
#else
	tpc->TxDescArray[entry].buf_addr = cpu_to_le32(
		pci_mem_to_phys(dev, (pci_addr_t)(unsigned long)ptxb));
#endif
	if (entry != (NUM_TX_DESC - 1)) {
		tpc->TxDescArray[entry].status =
			cpu_to_le32((OWNbit | FSbit | LSbit) |
				    ((len > ETH_ZLEN) ? len : ETH_ZLEN));
	} else {
		tpc->TxDescArray[entry].status =
			cpu_to_le32((OWNbit | EORbit | FSbit | LSbit) |
				    ((len > ETH_ZLEN) ? len : ETH_ZLEN));
	}
	rtl_flush_tx_desc(&tpc->TxDescArray[entry]);
	RTL_W8(TxPoll, 0x40);	/* set polling bit */

	tpc->cur_tx++;
	to = currticks() + TX_TIMEOUT;
	do {
		rtl_inval_tx_desc(&tpc->TxDescArray[entry]);
	} while ((le32_to_cpu(tpc->TxDescArray[entry].status) & OWNbit)
				&& (currticks() < to));	/* wait */

	if (currticks() >= to) {
#ifdef DEBUG_RTL8169_TX
		puts("tx timeout/error\n");
		printf("%s elapsed time : %lu\n", __func__, currticks()-stime);
#endif
		ret = -ETIMEDOUT;
	} else {
#ifdef DEBUG_RTL8169_TX
		puts("tx done\n");
#endif
		ret = 0;
	}
	/* Delay to make net console (nc) work properly */
	udelay(20);
	return ret;
}

#ifdef CONFIG_DM_ETH
int rtl8169_eth_send(struct udevice *dev, void *packet, int length)
{
	struct rtl8169_private *priv = dev_get_priv(dev);

	return rtl_send_common(dev, priv->iobase, packet, length);
}

#else
static int rtl_send(struct eth_device *dev, void *packet, int length)
{
	return rtl_send_common((pci_dev_t)(unsigned long)dev->priv,
			       dev->iobase, packet, length);
}
#endif

static void rtl8169_set_rx_mode(void)
{
	u32 mc_filter[2];	/* Multicast hash filter */
	int rx_mode;
	u32 tmp = 0;

#ifdef DEBUG_RTL8169
	printf ("%s\n", __FUNCTION__);
#endif

	/* IFF_ALLMULTI */
	/* Too many to filter perfectly -- accept all multicasts. */
	rx_mode = AcceptBroadcast | AcceptMulticast | AcceptMyPhys;
	mc_filter[1] = mc_filter[0] = 0xffffffff;

	tmp = rtl8169_rx_config | rx_mode | (RTL_R32(RxConfig) &
				   rtl_chip_info[tpc->chipset].RxConfigMask);

	RTL_W32(RxConfig, tmp);
	RTL_W32(MAR0 + 0, mc_filter[0]);
	RTL_W32(MAR0 + 4, mc_filter[1]);
}

#ifdef CONFIG_DM_ETH
static void rtl8169_hw_start(struct udevice *dev)
#else
static void rtl8169_hw_start(pci_dev_t dev)
#endif
{
	u32 i;

#ifdef DEBUG_RTL8169
	int stime = currticks();
	printf ("%s\n", __FUNCTION__);
#endif

#if 0
	/* Soft reset the chip. */
	RTL_W8(ChipCmd, CmdReset);

	/* Check that the chip has finished the reset. */
	for (i = 1000; i > 0; i--) {
		if ((RTL_R8(ChipCmd) & CmdReset) == 0)
			break;
		else
			udelay(10);
	}
#endif

	RTL_W8(Cfg9346, Cfg9346_Unlock);

	/* RTL-8169sb/8110sb or previous version */
	if (tpc->chipset <= 5)
		RTL_W8(ChipCmd, CmdTxEnb | CmdRxEnb);

	RTL_W8(EarlyTxThres, EarlyTxThld);

	/* For gigabit rtl8169 */
	RTL_W16(RxMaxSize, RxPacketMaxSize);

	/* Set Rx Config register */
	i = rtl8169_rx_config | (RTL_R32(RxConfig) &
				 rtl_chip_info[tpc->chipset].RxConfigMask);
	RTL_W32(RxConfig, i);

	/* Set DMA burst size and Interframe Gap Time */
	RTL_W32(TxConfig, (TX_DMA_BURST << TxDMAShift) |
				(InterFrameGap << TxInterFrameGapShift));


	tpc->cur_rx = 0;

#ifdef CONFIG_DM_ETH
	RTL_W32(TxDescStartAddrLow, dm_pci_mem_to_phys(dev,
			(pci_addr_t)(unsigned long)tpc->TxDescArray));
#else
	RTL_W32(TxDescStartAddrLow, pci_mem_to_phys(dev,
			(pci_addr_t)(unsigned long)tpc->TxDescArray));
#endif
	RTL_W32(TxDescStartAddrHigh, (unsigned long)0);
#ifdef CONFIG_DM_ETH
	RTL_W32(RxDescStartAddrLow, dm_pci_mem_to_phys(
			dev, (pci_addr_t)(unsigned long)tpc->RxDescArray));
#else
	RTL_W32(RxDescStartAddrLow, pci_mem_to_phys(
			dev, (pci_addr_t)(unsigned long)tpc->RxDescArray));
#endif
	RTL_W32(RxDescStartAddrHigh, (unsigned long)0);

	/* RTL-8169sc/8110sc or later version */
	if (tpc->chipset > 5)
		RTL_W8(ChipCmd, CmdTxEnb | CmdRxEnb);

	RTL_W8(Cfg9346, Cfg9346_Lock);
	udelay(10);

	RTL_W32(RxMissed, 0);

	rtl8169_set_rx_mode();

	/* no early-rx interrupts */
	RTL_W16(MultiIntr, RTL_R16(MultiIntr) & 0xF000);

#ifdef DEBUG_RTL8169
	printf("%s elapsed time : %lu\n", __func__, currticks()-stime);
#endif
}

#ifdef CONFIG_DM_ETH
static void rtl8169_init_ring(struct udevice *dev)
#else
static void rtl8169_init_ring(pci_dev_t dev)
#endif
{
	int i;

#ifdef DEBUG_RTL8169
	int stime = currticks();
	printf ("%s\n", __FUNCTION__);
#endif

	tpc->cur_rx = 0;
	tpc->cur_tx = 0;
	tpc->dirty_tx = 0;
	memset(tpc->TxDescArray, 0x0, NUM_TX_DESC * sizeof(struct TxDesc));
	memset(tpc->RxDescArray, 0x0, NUM_RX_DESC * sizeof(struct RxDesc));

	for (i = 0; i < NUM_TX_DESC; i++) {
		tpc->Tx_skbuff[i] = &txb[i];
	}

	for (i = 0; i < NUM_RX_DESC; i++) {
		if (i == (NUM_RX_DESC - 1))
			tpc->RxDescArray[i].status =
				cpu_to_le32((OWNbit | EORbit) + RX_BUF_SIZE);
		else
			tpc->RxDescArray[i].status =
				cpu_to_le32(OWNbit + RX_BUF_SIZE);

		tpc->RxBufferRing[i] = &rxb[i * RX_BUF_SIZE];
#ifdef CONFIG_DM_ETH
		tpc->RxDescArray[i].buf_addr = cpu_to_le32(dm_pci_mem_to_phys(
			dev, (pci_addr_t)(unsigned long)tpc->RxBufferRing[i]));
#else
		tpc->RxDescArray[i].buf_addr = cpu_to_le32(pci_mem_to_phys(
			dev, (pci_addr_t)(unsigned long)tpc->RxBufferRing[i]));
#endif
		rtl_flush_rx_desc(&tpc->RxDescArray[i]);
	}

#ifdef DEBUG_RTL8169
	printf("%s elapsed time : %lu\n", __func__, currticks()-stime);
#endif
}

#ifdef CONFIG_DM_ETH
static void rtl8169_common_start(struct udevice *dev, unsigned char *enetaddr,
				 unsigned long dev_iobase)
#else
static void rtl8169_common_start(pci_dev_t dev, unsigned char *enetaddr,
				 unsigned long dev_iobase)
#endif
{
	int i;

#ifdef DEBUG_RTL8169
	int stime = currticks();
	printf ("%s\n", __FUNCTION__);
#endif

	ioaddr = dev_iobase;

	rtl8169_init_ring(dev);
	rtl8169_hw_start(dev);
	/* Construct a perfect filter frame with the mac address as first match
	 * and broadcast for all others */
	for (i = 0; i < 192; i++)
		txb[i] = 0xFF;

	txb[0] = enetaddr[0];
	txb[1] = enetaddr[1];
	txb[2] = enetaddr[2];
	txb[3] = enetaddr[3];
	txb[4] = enetaddr[4];
	txb[5] = enetaddr[5];

#ifdef DEBUG_RTL8169
	printf("%s elapsed time : %lu\n", __func__, currticks()-stime);
#endif
}

#ifdef CONFIG_DM_ETH
static int rtl8169_eth_start(struct udevice *dev)
{
	struct eth_pdata *plat = dev_get_platdata(dev);
	struct rtl8169_private *priv = dev_get_priv(dev);

	rtl8169_common_start(dev, plat->enetaddr, priv->iobase);

	return 0;
}
#else
/**************************************************************************
RESET - Finish setting up the ethernet interface
***************************************************************************/
static int rtl_reset(struct eth_device *dev, bd_t *bis)
{
	rtl8169_common_start((pci_dev_t)(unsigned long)dev->priv,
			     dev->enetaddr, dev->iobase);

	return 0;
}
#endif /* nCONFIG_DM_ETH */

static void rtl_halt_common(unsigned long dev_iobase)
{
	int i;

#ifdef DEBUG_RTL8169
	printf ("%s\n", __FUNCTION__);
#endif

	ioaddr = dev_iobase;

	/* Stop the chip's Tx and Rx DMA processes. */
	RTL_W8(ChipCmd, 0x00);

	/* Disable interrupts by clearing the interrupt mask. */
	RTL_W16(IntrMask, 0x0000);

	RTL_W32(RxMissed, 0);

	for (i = 0; i < NUM_RX_DESC; i++) {
		tpc->RxBufferRing[i] = NULL;
	}
}

#ifdef CONFIG_DM_ETH
void rtl8169_eth_stop(struct udevice *dev)
{
	struct rtl8169_private *priv = dev_get_priv(dev);

	rtl_halt_common(priv->iobase);
}
#else
/**************************************************************************
HALT - Turn off ethernet interface
***************************************************************************/
static void rtl_halt(struct eth_device *dev)
{
	rtl_halt_common(dev->iobase);
}
#endif

#ifdef CONFIG_DM_ETH
static int rtl8169_write_hwaddr(struct udevice *dev)
{
	struct eth_pdata *plat = dev_get_platdata(dev);
	unsigned int i;

	RTL_W8(Cfg9346, Cfg9346_Unlock);

	for (i = 0; i < MAC_ADDR_LEN; i++)
		RTL_W8(MAC0 + i, plat->enetaddr[i]);

	RTL_W8(Cfg9346, Cfg9346_Lock);

	return 0;
}
#endif

/**************************************************************************
INIT - Look for an adapter, this routine's visible to the outside
***************************************************************************/

#define board_found 1
#define valid_link 0
static int rtl_init(unsigned long dev_ioaddr, const char *name,
		    unsigned char *enetaddr)
{
	static int board_idx = -1;
	int i, rc;
	int option = -1, Cap10_100 = 0, Cap1000 = 0;

#ifdef DEBUG_RTL8169
	printf ("%s\n", __FUNCTION__);
#endif
	ioaddr = dev_ioaddr;

	board_idx++;

	/* point to private storage */
	tpc = &tpx;

	rc = rtl8169_init_board(ioaddr, name);
	if (rc)
		return rc;

	/* Get MAC address.  FIXME: read EEPROM */
	for (i = 0; i < MAC_ADDR_LEN; i++)
		enetaddr[i] = RTL_R8(MAC0 + i);

#ifdef DEBUG_RTL8169
	printf("chipset = %d\n", tpc->chipset);
	printf("MAC Address");
	for (i = 0; i < MAC_ADDR_LEN; i++)
		printf(":%02x", enetaddr[i]);
	putc('\n');
#endif

#ifdef DEBUG_RTL8169
	/* Print out some hardware info */
	printf("%s: at ioaddr 0x%lx\n", name, ioaddr);
#endif

	/* if TBI is not endbled */
	if (!(RTL_R8(PHYstatus) & TBI_Enable)) {
		int val = mdio_read(PHY_AUTO_NEGO_REG);

		option = (board_idx >= MAX_UNITS) ? 0 : media[board_idx];
		/* Force RTL8169 in 10/100/1000 Full/Half mode. */
		if (option > 0) {
#ifdef DEBUG_RTL8169
			printf("%s: Force-mode Enabled.\n", name);
#endif
			Cap10_100 = 0, Cap1000 = 0;
			switch (option) {
			case _10_Half:
				Cap10_100 = PHY_Cap_10_Half;
				Cap1000 = PHY_Cap_Null;
				break;
			case _10_Full:
				Cap10_100 = PHY_Cap_10_Full;
				Cap1000 = PHY_Cap_Null;
				break;
			case _100_Half:
				Cap10_100 = PHY_Cap_100_Half;
				Cap1000 = PHY_Cap_Null;
				break;
			case _100_Full:
				Cap10_100 = PHY_Cap_100_Full;
				Cap1000 = PHY_Cap_Null;
				break;
			case _1000_Full:
				Cap10_100 = PHY_Cap_Null;
				Cap1000 = PHY_Cap_1000_Full;
				break;
			default:
				break;
			}
			mdio_write(PHY_AUTO_NEGO_REG, Cap10_100 | (val & 0x1F));	/* leave PHY_AUTO_NEGO_REG bit4:0 unchanged */
			mdio_write(PHY_1000_CTRL_REG, Cap1000);
		} else {
#ifdef DEBUG_RTL8169
			printf("%s: Auto-negotiation Enabled.\n",
			       name);
#endif
			/* enable 10/100 Full/Half Mode, leave PHY_AUTO_NEGO_REG bit4:0 unchanged */
			mdio_write(PHY_AUTO_NEGO_REG,
				   PHY_Cap_10_Half | PHY_Cap_10_Full |
				   PHY_Cap_100_Half | PHY_Cap_100_Full |
				   (val & 0x1F));

			/* enable 1000 Full Mode */
			mdio_write(PHY_1000_CTRL_REG, PHY_Cap_1000_Full);

		}

		/* Enable auto-negotiation and restart auto-nigotiation */
		mdio_write(PHY_CTRL_REG,
			   PHY_Enable_Auto_Nego | PHY_Restart_Auto_Nego);
		udelay(100);

		/* wait for auto-negotiation process */
		for (i = 10000; i > 0; i--) {
			/* check if auto-negotiation complete */
			if (mdio_read(PHY_STAT_REG) & PHY_Auto_Nego_Comp) {
				udelay(100);
				option = RTL_R8(PHYstatus);
				if (option & _1000bpsF) {
#ifdef DEBUG_RTL8169
					printf("%s: 1000Mbps Full-duplex operation.\n",
					       name);
#endif
				} else {
#ifdef DEBUG_RTL8169
					printf("%s: %sMbps %s-duplex operation.\n",
					       name,
					       (option & _100bps) ? "100" :
					       "10",
					       (option & FullDup) ? "Full" :
					       "Half");
#endif
				}
				break;
			} else {
				udelay(100);
			}
		}		/* end for-loop to wait for auto-negotiation process */

	} else {
		udelay(100);
#ifdef DEBUG_RTL8169
		printf
		    ("%s: 1000Mbps Full-duplex operation, TBI Link %s!\n",
		     name,
		     (RTL_R32(TBICSR) & TBILinkOK) ? "OK" : "Failed");
#endif
	}


	tpc->RxDescArray = rtl_alloc_descs(NUM_RX_DESC);
	if (!tpc->RxDescArray)
		return -ENOMEM;

	tpc->TxDescArray = rtl_alloc_descs(NUM_TX_DESC);
	if (!tpc->TxDescArray)
		return -ENOMEM;

	return 0;
}

#ifndef CONFIG_DM_ETH
int rtl8169_initialize(bd_t *bis)
{
	pci_dev_t devno;
	int card_number = 0;
	struct eth_device *dev;
	u32 iobase;
	int idx=0;

	while(1){
		unsigned int region;
		u16 device;
		int err;

		/* Find RTL8169 */
		if ((devno = pci_find_devices(supported, idx++)) < 0)
			break;

		pci_read_config_word(devno, PCI_DEVICE_ID, &device);
		switch (device) {
		case 0x8168:
			region = 2;
			break;

		default:
			region = 1;
			break;
		}

		pci_read_config_dword(devno, PCI_BASE_ADDRESS_0 + (region * 4), &iobase);
		iobase &= ~0xf;

		debug ("rtl8169: REALTEK RTL8169 @0x%x\n", iobase);

		dev = (struct eth_device *)malloc(sizeof *dev);
		if (!dev) {
			printf("Can not allocate memory of rtl8169\n");
			break;
		}

		memset(dev, 0, sizeof(*dev));
		sprintf (dev->name, "RTL8169#%d", card_number);

		dev->priv = (void *)(unsigned long)devno;
		dev->iobase = (int)pci_mem_to_phys(devno, iobase);

		dev->init = rtl_reset;
		dev->halt = rtl_halt;
		dev->send = rtl_send;
		dev->recv = rtl_recv;

		err = rtl_init(dev->iobase, dev->name, dev->enetaddr);
		if (err < 0) {
			printf(pr_fmt("failed to initialize card: %d\n"), err);
			free(dev);
			continue;
		}

		eth_register (dev);

		card_number++;
	}
	return card_number;
}
#endif

#ifdef CONFIG_DM_ETH
static int rtl8169_eth_probe(struct udevice *dev)
{
	struct pci_child_platdata *pplat = dev_get_parent_platdata(dev);
	struct rtl8169_private *priv = dev_get_priv(dev);
	struct eth_pdata *plat = dev_get_platdata(dev);
	u32 iobase;
	int region;
	int ret;

	debug("rtl8169: REALTEK RTL8169 @0x%x\n", iobase);
	switch (pplat->device) {
	case 0x8168:
		region = 2;
		break;
	default:
		region = 1;
		break;
	}
	dm_pci_read_config32(dev, PCI_BASE_ADDRESS_0 + region * 4, &iobase);
	iobase &= ~0xf;
	priv->iobase = (int)dm_pci_mem_to_phys(dev, iobase);

	ret = rtl_init(priv->iobase, dev->name, plat->enetaddr);
	if (ret < 0) {
		printf(pr_fmt("failed to initialize card: %d\n"), ret);
		return ret;
	}

	return 0;
}

static const struct eth_ops rtl8169_eth_ops = {
	.start	= rtl8169_eth_start,
	.send	= rtl8169_eth_send,
	.recv	= rtl8169_eth_recv,
	.stop	= rtl8169_eth_stop,
	.write_hwaddr = rtl8169_write_hwaddr,
};

static const struct udevice_id rtl8169_eth_ids[] = {
	{ .compatible = "realtek,rtl8169" },
	{ }
};

U_BOOT_DRIVER(eth_rtl8169) = {
	.name	= "eth_rtl8169",
	.id	= UCLASS_ETH,
	.of_match = rtl8169_eth_ids,
	.probe	= rtl8169_eth_probe,
	.ops	= &rtl8169_eth_ops,
	.priv_auto_alloc_size = sizeof(struct rtl8169_private),
	.platdata_auto_alloc_size = sizeof(struct eth_pdata),
};

U_BOOT_PCI_DEVICE(eth_rtl8169, supported);
#endif
