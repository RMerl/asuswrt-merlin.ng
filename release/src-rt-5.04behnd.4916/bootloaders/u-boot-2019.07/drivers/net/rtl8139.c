/*
 * rtl8139.c : U-Boot driver for the RealTek RTL8139
 *
 * Masami Komiya (mkomiya@sonare.it)
 *
 * Most part is taken from rtl8139.c of etherboot
 *
 */

/* rtl8139.c - etherboot driver for the Realtek 8139 chipset

  ported from the linux driver written by Donald Becker
  by Rainer Bawidamann (Rainer.Bawidamann@informatik.uni-ulm.de) 1999

  This software may be used and distributed according to the terms
  of the GNU Public License, incorporated herein by reference.

  changes to the original driver:
  - removed support for interrupts, switching to polling mode (yuck!)
  - removed support for the 8129 chip (external MII)

*/

/*********************************************************************/
/* Revision History						     */
/*********************************************************************/

/*
  28 Dec 2002	ken_yap@users.sourceforge.net (Ken Yap)
     Put in virt_to_bus calls to allow Etherboot relocation.

  06 Apr 2001	ken_yap@users.sourceforge.net (Ken Yap)
     Following email from Hyun-Joon Cha, added a disable routine, otherwise
     NIC remains live and can crash the kernel later.

  4 Feb 2000	espenlaub@informatik.uni-ulm.de (Klaus Espenlaub)
     Shuffled things around, removed the leftovers from the 8129 support
     that was in the Linux driver and added a bit more 8139 definitions.
     Moved the 8K receive buffer to a fixed, available address outside the
     0x98000-0x9ffff range.  This is a bit of a hack, but currently the only
     way to make room for the Etherboot features that need substantial amounts
     of code like the ANSI console support.  Currently the buffer is just below
     0x10000, so this even conforms to the tagged boot image specification,
     which reserves the ranges 0x00000-0x10000 and 0x98000-0xA0000.  My
     interpretation of this "reserved" is that Etherboot may do whatever it
     likes, as long as its environment is kept intact (like the BIOS
     variables).  Hopefully fixed rtl_poll() once and for all.	The symptoms
     were that if Etherboot was left at the boot menu for several minutes, the
     first eth_poll failed.  Seems like I am the only person who does this.
     First of all I fixed the debugging code and then set out for a long bug
     hunting session.  It took me about a week full time work - poking around
     various places in the driver, reading Don Becker's and Jeff Garzik's Linux
     driver and even the FreeBSD driver (what a piece of crap!) - and
     eventually spotted the nasty thing: the transmit routine was acknowledging
     each and every interrupt pending, including the RxOverrun and RxFIFIOver
     interrupts.  This confused the RTL8139 thoroughly.	 It destroyed the
     Rx ring contents by dumping the 2K FIFO contents right where we wanted to
     get the next packet.  Oh well, what fun.

  18 Jan 2000	mdc@thinguin.org (Marty Connor)
     Drastically simplified error handling.  Basically, if any error
     in transmission or reception occurs, the card is reset.
     Also, pointed all transmit descriptors to the same buffer to
     save buffer space.	 This should decrease driver size and avoid
     corruption because of exceeding 32K during runtime.

  28 Jul 1999	(Matthias Meixner - meixner@rbg.informatik.tu-darmstadt.de)
     rtl_poll was quite broken: it used the RxOK interrupt flag instead
     of the RxBufferEmpty flag which often resulted in very bad
     transmission performace - below 1kBytes/s.

*/

#include <common.h>
#include <malloc.h>
#include <net.h>
#include <netdev.h>
#include <asm/io.h>
#include <pci.h>

#define RTL_TIMEOUT	100000

/* PCI Tuning Parameters
   Threshold is bytes transferred to chip before transmission starts. */
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

#define bus_to_phys(a)	pci_mem_to_phys((pci_dev_t)dev->priv, a)
#define phys_to_bus(a)	pci_phys_to_mem((pci_dev_t)dev->priv, a)

/* Symbolic offsets to registers. */
enum RTL8139_registers {
	MAC0=0,			/* Ethernet hardware address. */
	MAR0=8,			/* Multicast filter. */
	TxStatus0=0x10,		/* Transmit status (four 32bit registers). */
	TxAddr0=0x20,		/* Tx descriptors (also four 32bit). */
	RxBuf=0x30, RxEarlyCnt=0x34, RxEarlyStatus=0x36,
	ChipCmd=0x37, RxBufPtr=0x38, RxBufAddr=0x3A,
	IntrMask=0x3C, IntrStatus=0x3E,
	TxConfig=0x40, RxConfig=0x44,
	Timer=0x48,		/* general-purpose counter. */
	RxMissed=0x4C,		/* 24 bits valid, write clears. */
	Cfg9346=0x50, Config0=0x51, Config1=0x52,
	TimerIntrReg=0x54,	/* intr if gp counter reaches this value */
	MediaStatus=0x58,
	Config3=0x59,
	MultiIntr=0x5C,
	RevisionID=0x5E,	/* revision of the RTL8139 chip */
	TxSummary=0x60,
	MII_BMCR=0x62, MII_BMSR=0x64, NWayAdvert=0x66, NWayLPAR=0x68,
	NWayExpansion=0x6A,
	DisconnectCnt=0x6C, FalseCarrierCnt=0x6E,
	NWayTestReg=0x70,
	RxCnt=0x72,		/* packet received counter */
	CSCR=0x74,		/* chip status and configuration register */
	PhyParm1=0x78,TwisterParm=0x7c,PhyParm2=0x80,	/* undocumented */
	/* from 0x84 onwards are a number of power management/wakeup frame
	 * definitions we will probably never need to know about.  */
};

enum ChipCmdBits {
	CmdReset=0x10, CmdRxEnb=0x08, CmdTxEnb=0x04, RxBufEmpty=0x01, };

/* Interrupt register bits, using my own meaningful names. */
enum IntrStatusBits {
	PCIErr=0x8000, PCSTimeout=0x4000, CableLenChange= 0x2000,
	RxFIFOOver=0x40, RxUnderrun=0x20, RxOverflow=0x10,
	TxErr=0x08, TxOK=0x04, RxErr=0x02, RxOK=0x01,
};
enum TxStatusBits {
	TxHostOwns=0x2000, TxUnderrun=0x4000, TxStatOK=0x8000,
	TxOutOfWindow=0x20000000, TxAborted=0x40000000,
	TxCarrierLost=0x80000000,
};
enum RxStatusBits {
	RxMulticast=0x8000, RxPhysical=0x4000, RxBroadcast=0x2000,
	RxBadSymbol=0x0020, RxRunt=0x0010, RxTooLong=0x0008, RxCRCErr=0x0004,
	RxBadAlign=0x0002, RxStatusOK=0x0001,
};

enum MediaStatusBits {
	MSRTxFlowEnable=0x80, MSRRxFlowEnable=0x40, MSRSpeed10=0x08,
	MSRLinkFail=0x04, MSRRxPauseFlag=0x02, MSRTxPauseFlag=0x01,
};

enum MIIBMCRBits {
	BMCRReset=0x8000, BMCRSpeed100=0x2000, BMCRNWayEnable=0x1000,
	BMCRRestartNWay=0x0200, BMCRDuplex=0x0100,
};

enum CSCRBits {
	CSCR_LinkOKBit=0x0400, CSCR_LinkChangeBit=0x0800,
	CSCR_LinkStatusBits=0x0f000, CSCR_LinkDownOffCmd=0x003c0,
	CSCR_LinkDownCmd=0x0f3c0,
};

/* Bits in RxConfig. */
enum rx_mode_bits {
	RxCfgWrap=0x80,
	AcceptErr=0x20, AcceptRunt=0x10, AcceptBroadcast=0x08,
	AcceptMulticast=0x04, AcceptMyPhys=0x02, AcceptAllPhys=0x01,
};

static int ioaddr;
static unsigned int cur_rx,cur_tx;

/* The RTL8139 can only transmit from a contiguous, aligned memory block.  */
static unsigned char tx_buffer[TX_BUF_SIZE] __attribute__((aligned(4)));
static unsigned char rx_ring[RX_BUF_LEN+16] __attribute__((aligned(4)));

static int rtl8139_probe(struct eth_device *dev, bd_t *bis);
static int read_eeprom(int location, int addr_len);
static void rtl_reset(struct eth_device *dev);
static int rtl_transmit(struct eth_device *dev, void *packet, int length);
static int rtl_poll(struct eth_device *dev);
static void rtl_disable(struct eth_device *dev);
static int rtl_bcast_addr(struct eth_device *dev, const u8 *bcast_mac, int join)
{
	return (0);
}

static struct pci_device_id supported[] = {
       {PCI_VENDOR_ID_REALTEK, PCI_DEVICE_ID_REALTEK_8139},
       {PCI_VENDOR_ID_DLINK, PCI_DEVICE_ID_DLINK_8139},
       {}
};

int rtl8139_initialize(bd_t *bis)
{
	pci_dev_t devno;
	int card_number = 0;
	struct eth_device *dev;
	u32 iobase;
	int idx=0;

	while(1){
		/* Find RTL8139 */
		if ((devno = pci_find_devices(supported, idx++)) < 0)
			break;

		pci_read_config_dword(devno, PCI_BASE_ADDRESS_1, &iobase);
		iobase &= ~0xf;

		debug ("rtl8139: REALTEK RTL8139 @0x%x\n", iobase);

		dev = (struct eth_device *)malloc(sizeof *dev);
		if (!dev) {
			printf("Can not allocate memory of rtl8139\n");
			break;
		}
		memset(dev, 0, sizeof(*dev));

		sprintf (dev->name, "RTL8139#%d", card_number);

		dev->priv = (void *) devno;
		dev->iobase = (int)bus_to_phys(iobase);
		dev->init = rtl8139_probe;
		dev->halt = rtl_disable;
		dev->send = rtl_transmit;
		dev->recv = rtl_poll;
		dev->mcast = rtl_bcast_addr;

		eth_register (dev);

		card_number++;

		pci_write_config_byte (devno, PCI_LATENCY_TIMER, 0x20);

		udelay (10 * 1000);
	}

	return card_number;
}

static int rtl8139_probe(struct eth_device *dev, bd_t *bis)
{
	int i;
	int addr_len;
	unsigned short *ap = (unsigned short *)dev->enetaddr;

	ioaddr = dev->iobase;

	/* Bring the chip out of low-power mode. */
	outb(0x00, ioaddr + Config1);

	addr_len = read_eeprom(0,8) == 0x8129 ? 8 : 6;
	for (i = 0; i < 3; i++)
		*ap++ = le16_to_cpu (read_eeprom(i + 7, addr_len));

	rtl_reset(dev);

	if (inb(ioaddr + MediaStatus) & MSRLinkFail) {
		printf("Cable not connected or other link failure\n");
		return -1 ;
	}

	return 0;
}

/* Serial EEPROM section. */

/*  EEPROM_Ctrl bits. */
#define EE_SHIFT_CLK	0x04	/* EEPROM shift clock. */
#define EE_CS		0x08	/* EEPROM chip select. */
#define EE_DATA_WRITE	0x02	/* EEPROM chip data in. */
#define EE_WRITE_0	0x00
#define EE_WRITE_1	0x02
#define EE_DATA_READ	0x01	/* EEPROM chip data out. */
#define EE_ENB		(0x80 | EE_CS)

/*
	Delay between EEPROM clock transitions.
	No extra delay is needed with 33MHz PCI, but 66MHz may change this.
*/

#define eeprom_delay()	inl(ee_addr)

/* The EEPROM commands include the alway-set leading bit. */
#define EE_WRITE_CMD	(5)
#define EE_READ_CMD	(6)
#define EE_ERASE_CMD	(7)

static int read_eeprom(int location, int addr_len)
{
	int i;
	unsigned int retval = 0;
	long ee_addr = ioaddr + Cfg9346;
	int read_cmd = location | (EE_READ_CMD << addr_len);

	outb(EE_ENB & ~EE_CS, ee_addr);
	outb(EE_ENB, ee_addr);
	eeprom_delay();

	/* Shift the read command bits out. */
	for (i = 4 + addr_len; i >= 0; i--) {
		int dataval = (read_cmd & (1 << i)) ? EE_DATA_WRITE : 0;
		outb(EE_ENB | dataval, ee_addr);
		eeprom_delay();
		outb(EE_ENB | dataval | EE_SHIFT_CLK, ee_addr);
		eeprom_delay();
	}
	outb(EE_ENB, ee_addr);
	eeprom_delay();

	for (i = 16; i > 0; i--) {
		outb(EE_ENB | EE_SHIFT_CLK, ee_addr);
		eeprom_delay();
		retval = (retval << 1) | ((inb(ee_addr) & EE_DATA_READ) ? 1 : 0);
		outb(EE_ENB, ee_addr);
		eeprom_delay();
	}

	/* Terminate the EEPROM access. */
	outb(~EE_CS, ee_addr);
	eeprom_delay();
	return retval;
}

static const unsigned int rtl8139_rx_config =
	(RX_BUF_LEN_IDX << 11) |
	(RX_FIFO_THRESH << 13) |
	(RX_DMA_BURST << 8);

static void set_rx_mode(struct eth_device *dev) {
	unsigned int mc_filter[2];
	int rx_mode;
	/* !IFF_PROMISC */
	rx_mode = AcceptBroadcast | AcceptMulticast | AcceptMyPhys;
	mc_filter[1] = mc_filter[0] = 0xffffffff;

	outl(rtl8139_rx_config | rx_mode, ioaddr + RxConfig);

	outl(mc_filter[0], ioaddr + MAR0 + 0);
	outl(mc_filter[1], ioaddr + MAR0 + 4);
}

static void rtl_reset(struct eth_device *dev)
{
	int i;

	outb(CmdReset, ioaddr + ChipCmd);

	cur_rx = 0;
	cur_tx = 0;

	/* Give the chip 10ms to finish the reset. */
	for (i=0; i<100; ++i){
		if ((inb(ioaddr + ChipCmd) & CmdReset) == 0) break;
		udelay (100); /* wait 100us */
	}


	for (i = 0; i < ETH_ALEN; i++)
		outb(dev->enetaddr[i], ioaddr + MAC0 + i);

	/* Must enable Tx/Rx before setting transfer thresholds! */
	outb(CmdRxEnb | CmdTxEnb, ioaddr + ChipCmd);
	outl((RX_FIFO_THRESH<<13) | (RX_BUF_LEN_IDX<<11) | (RX_DMA_BURST<<8),
		ioaddr + RxConfig);		/* accept no frames yet!  */
	outl((TX_DMA_BURST<<8)|0x03000000, ioaddr + TxConfig);

	/* The Linux driver changes Config1 here to use a different LED pattern
	 * for half duplex or full/autodetect duplex (for full/autodetect, the
	 * outputs are TX/RX, Link10/100, FULL, while for half duplex it uses
	 * TX/RX, Link100, Link10).  This is messy, because it doesn't match
	 * the inscription on the mounting bracket.  It should not be changed
	 * from the configuration EEPROM default, because the card manufacturer
	 * should have set that to match the card.  */

	debug_cond(DEBUG_RX,
		"rx ring address is %lX\n",(unsigned long)rx_ring);
	flush_cache((unsigned long)rx_ring, RX_BUF_LEN);
	outl(phys_to_bus((int)rx_ring), ioaddr + RxBuf);

	/* If we add multicast support, the MAR0 register would have to be
	 * initialized to 0xffffffffffffffff (two 32 bit accesses).  Etherboot
	 * only needs broadcast (for ARP/RARP/BOOTP/DHCP) and unicast.	*/

	outb(CmdRxEnb | CmdTxEnb, ioaddr + ChipCmd);

	outl(rtl8139_rx_config, ioaddr + RxConfig);

	/* Start the chip's Tx and Rx process. */
	outl(0, ioaddr + RxMissed);

	/* set_rx_mode */
	set_rx_mode(dev);

	/* Disable all known interrupts by setting the interrupt mask. */
	outw(0, ioaddr + IntrMask);
}

static int rtl_transmit(struct eth_device *dev, void *packet, int length)
{
	unsigned int status;
	unsigned long txstatus;
	unsigned int len = length;
	int i = 0;

	ioaddr = dev->iobase;

	memcpy((char *)tx_buffer, (char *)packet, (int)length);

	debug_cond(DEBUG_TX, "sending %d bytes\n", len);

	/* Note: RTL8139 doesn't auto-pad, send minimum payload (another 4
	 * bytes are sent automatically for the FCS, totalling to 64 bytes). */
	while (len < ETH_ZLEN) {
		tx_buffer[len++] = '\0';
	}

	flush_cache((unsigned long)tx_buffer, length);
	outl(phys_to_bus((int)tx_buffer), ioaddr + TxAddr0 + cur_tx*4);
	outl(((TX_FIFO_THRESH<<11) & 0x003f0000) | len,
		ioaddr + TxStatus0 + cur_tx*4);

	do {
		status = inw(ioaddr + IntrStatus);
		/* Only acknlowledge interrupt sources we can properly handle
		 * here - the RxOverflow/RxFIFOOver MUST be handled in the
		 * rtl_poll() function.	 */
		outw(status & (TxOK | TxErr | PCIErr), ioaddr + IntrStatus);
		if ((status & (TxOK | TxErr | PCIErr)) != 0) break;
		udelay(10);
	} while (i++ < RTL_TIMEOUT);

	txstatus = inl(ioaddr + TxStatus0 + cur_tx*4);

	if (status & TxOK) {
		cur_tx = (cur_tx + 1) % NUM_TX_DESC;

		debug_cond(DEBUG_TX,
			"tx done, status %hX txstatus %lX\n",
			status, txstatus);

		return length;
	} else {

		debug_cond(DEBUG_TX,
			"tx timeout/error (%d usecs), status %hX txstatus %lX\n",
			10*i, status, txstatus);

		rtl_reset(dev);

		return 0;
	}
}

static int rtl_poll(struct eth_device *dev)
{
	unsigned int status;
	unsigned int ring_offs;
	unsigned int rx_size, rx_status;
	int length=0;

	ioaddr = dev->iobase;

	if (inb(ioaddr + ChipCmd) & RxBufEmpty) {
		return 0;
	}

	status = inw(ioaddr + IntrStatus);
	/* See below for the rest of the interrupt acknowledges.  */
	outw(status & ~(RxFIFOOver | RxOverflow | RxOK), ioaddr + IntrStatus);

	debug_cond(DEBUG_RX, "rtl_poll: int %hX ", status);

	ring_offs = cur_rx % RX_BUF_LEN;
	/* ring_offs is guaranteed being 4-byte aligned */
	rx_status = le32_to_cpu(*(unsigned int *)(rx_ring + ring_offs));
	rx_size = rx_status >> 16;
	rx_status &= 0xffff;

	if ((rx_status & (RxBadSymbol|RxRunt|RxTooLong|RxCRCErr|RxBadAlign)) ||
	    (rx_size < ETH_ZLEN) || (rx_size > ETH_FRAME_LEN + 4)) {
		printf("rx error %hX\n", rx_status);
		rtl_reset(dev); /* this clears all interrupts still pending */
		return 0;
	}

	/* Received a good packet */
	length = rx_size - 4;	/* no one cares about the FCS */
	if (ring_offs+4+rx_size-4 > RX_BUF_LEN) {
		int semi_count = RX_BUF_LEN - ring_offs - 4;
		unsigned char rxdata[RX_BUF_LEN];

		memcpy(rxdata, rx_ring + ring_offs + 4, semi_count);
		memcpy(&(rxdata[semi_count]), rx_ring, rx_size-4-semi_count);

		net_process_received_packet(rxdata, length);
		debug_cond(DEBUG_RX, "rx packet %d+%d bytes",
			semi_count, rx_size-4-semi_count);
	} else {
		net_process_received_packet(rx_ring + ring_offs + 4, length);
		debug_cond(DEBUG_RX, "rx packet %d bytes", rx_size-4);
	}
	flush_cache((unsigned long)rx_ring, RX_BUF_LEN);

	cur_rx = (cur_rx + rx_size + 4 + 3) & ~3;
	outw(cur_rx - 16, ioaddr + RxBufPtr);
	/* See RTL8139 Programming Guide V0.1 for the official handling of
	 * Rx overflow situations.  The document itself contains basically no
	 * usable information, except for a few exception handling rules.  */
	outw(status & (RxFIFOOver | RxOverflow | RxOK), ioaddr + IntrStatus);
	return length;
}

static void rtl_disable(struct eth_device *dev)
{
	int i;

	ioaddr = dev->iobase;

	/* reset the chip */
	outb(CmdReset, ioaddr + ChipCmd);

	/* Give the chip 10ms to finish the reset. */
	for (i=0; i<100; ++i){
		if ((inb(ioaddr + ChipCmd) & CmdReset) == 0) break;
		udelay (100); /* wait 100us */
	}
}
