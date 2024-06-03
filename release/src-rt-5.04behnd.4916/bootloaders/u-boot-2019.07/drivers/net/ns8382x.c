/*
   ns8382x.c: A U-Boot driver for the NatSemi DP8382[01].
   ported by: Mark A. Rakes (mark_rakes@vivato.net)

   Adapted from:
   1. an Etherboot driver for DP8381[56] written by:
	   Copyright (C) 2001 Entity Cyber, Inc.

	   This development of this Etherboot driver was funded by
		  Sicom Systems: http://www.sicompos.com/

	   Author: Marty Connor (mdc@thinguin.org)
	   Adapted from a Linux driver which was written by Donald Becker

	   This software may be used and distributed according to the terms
	   of the GNU Public License (GPL), incorporated herein by reference.

   2. A Linux driver by Donald Becker, ns820.c:
		Written/copyright 1999-2002 by Donald Becker.

		This software may be used and distributed according to the terms of
		the GNU General Public License (GPL), incorporated herein by reference.
		Drivers based on or derived from this code fall under the GPL and must
		retain the authorship, copyright and license notice.  This file is not
		a complete program and may only be used when the entire operating
		system is licensed under the GPL.  License for under other terms may be
		available.  Contact the original author for details.

		The original author may be reached as becker@scyld.com, or at
		Scyld Computing Corporation
		410 Severn Ave., Suite 210
		Annapolis MD 21403

		Support information and updates available at
		http://www.scyld.com/network/netsemi.html

   Datasheets available from:
   http://www.national.com/pf/DP/DP83820.html
   http://www.national.com/pf/DP/DP83821.html
*/

/* Revision History
 * October 2002 mar	1.0
 *   Initial U-Boot Release.
 *	Tested with Netgear GA622T (83820)
 *	and SMC9452TX (83821)
 *	NOTE: custom boards with these chips may (likely) require
 *	a programmed EEPROM device (if present) in order to work
 *	correctly.
*/

/* Includes */
#include <common.h>
#include <malloc.h>
#include <net.h>
#include <netdev.h>
#include <asm/io.h>
#include <pci.h>

/* defines */
#define DSIZE     0x00000FFF
#define CRC_SIZE  4
#define TOUT_LOOP   500000
#define TX_BUF_SIZE    1536
#define RX_BUF_SIZE    1536
#define NUM_RX_DESC    4	/* Number of Rx descriptor registers. */

enum register_offsets {
	ChipCmd = 0x00,
	ChipConfig = 0x04,
	EECtrl = 0x08,
	IntrMask = 0x14,
	IntrEnable = 0x18,
	TxRingPtr = 0x20,
	TxRingPtrHi = 0x24,
	TxConfig = 0x28,
	RxRingPtr = 0x30,
	RxRingPtrHi = 0x34,
	RxConfig = 0x38,
	PriQueue = 0x3C,
	RxFilterAddr = 0x48,
	RxFilterData = 0x4C,
	ClkRun = 0xCC,
	PCIPM = 0x44,
};

enum ChipCmdBits {
	ChipReset = 0x100,
	RxReset = 0x20,
	TxReset = 0x10,
	RxOff = 0x08,
	RxOn = 0x04,
	TxOff = 0x02,
	TxOn = 0x01
};

enum ChipConfigBits {
	LinkSts = 0x80000000,
	GigSpeed = 0x40000000,
	HundSpeed = 0x20000000,
	FullDuplex = 0x10000000,
	TBIEn = 0x01000000,
	Mode1000 = 0x00400000,
	T64En = 0x00004000,
	D64En = 0x00001000,
	M64En = 0x00000800,
	PhyRst = 0x00000400,
	PhyDis = 0x00000200,
	ExtStEn = 0x00000100,
	BEMode = 0x00000001,
};
#define SpeedStatus_Polarity ( GigSpeed | HundSpeed | FullDuplex)

enum TxConfig_bits {
	TxDrthMask	= 0x000000ff,
	TxFlthMask	= 0x0000ff00,
	TxMxdmaMask	= 0x00700000,
	TxMxdma_8	= 0x00100000,
	TxMxdma_16	= 0x00200000,
	TxMxdma_32	= 0x00300000,
	TxMxdma_64	= 0x00400000,
	TxMxdma_128	= 0x00500000,
	TxMxdma_256	= 0x00600000,
	TxMxdma_512	= 0x00700000,
	TxMxdma_1024	= 0x00000000,
	TxCollRetry	= 0x00800000,
	TxAutoPad	= 0x10000000,
	TxMacLoop	= 0x20000000,
	TxHeartIgn	= 0x40000000,
	TxCarrierIgn	= 0x80000000
};

enum RxConfig_bits {
	RxDrthMask	= 0x0000003e,
	RxMxdmaMask	= 0x00700000,
	RxMxdma_8	= 0x00100000,
	RxMxdma_16	= 0x00200000,
	RxMxdma_32	= 0x00300000,
	RxMxdma_64	= 0x00400000,
	RxMxdma_128	= 0x00500000,
	RxMxdma_256	= 0x00600000,
	RxMxdma_512	= 0x00700000,
	RxMxdma_1024	= 0x00000000,
	RxAcceptLenErr	= 0x04000000,
	RxAcceptLong	= 0x08000000,
	RxAcceptTx	= 0x10000000,
	RxStripCRC	= 0x20000000,
	RxAcceptRunt	= 0x40000000,
	RxAcceptErr	= 0x80000000,
};

/* Bits in the RxMode register. */
enum rx_mode_bits {
	RxFilterEnable		= 0x80000000,
	AcceptAllBroadcast	= 0x40000000,
	AcceptAllMulticast	= 0x20000000,
	AcceptAllUnicast	= 0x10000000,
	AcceptPerfectMatch	= 0x08000000,
};

typedef struct _BufferDesc {
	u32 link;
	u32 bufptr;
	vu_long cmdsts;
	u32 extsts;		/*not used here */
} BufferDesc;

/* Bits in network_desc.status */
enum desc_status_bits {
	DescOwn = 0x80000000, DescMore = 0x40000000, DescIntr = 0x20000000,
	DescNoCRC = 0x10000000, DescPktOK = 0x08000000,
	DescSizeMask = 0xfff,

	DescTxAbort = 0x04000000, DescTxFIFO = 0x02000000,
	DescTxCarrier = 0x01000000, DescTxDefer = 0x00800000,
	DescTxExcDefer = 0x00400000, DescTxOOWCol = 0x00200000,
	DescTxExcColl = 0x00100000, DescTxCollCount = 0x000f0000,

	DescRxAbort = 0x04000000, DescRxOver = 0x02000000,
	DescRxDest = 0x01800000, DescRxLong = 0x00400000,
	DescRxRunt = 0x00200000, DescRxInvalid = 0x00100000,
	DescRxCRC = 0x00080000, DescRxAlign = 0x00040000,
	DescRxLoop = 0x00020000, DesRxColl = 0x00010000,
};

/* Bits in MEAR */
enum mii_reg_bits {
	MDIO_ShiftClk = 0x0040,
	MDIO_EnbOutput = 0x0020,
	MDIO_Data = 0x0010,
};

/* PHY Register offsets.  */
enum phy_reg_offsets {
	BMCR = 0x00,
	BMSR = 0x01,
	PHYIDR1 = 0x02,
	PHYIDR2 = 0x03,
	ANAR = 0x04,
	KTCR = 0x09,
};

/* basic mode control register bits */
enum bmcr_bits {
	Bmcr_Reset = 0x8000,
	Bmcr_Loop = 0x4000,
	Bmcr_Speed0 = 0x2000,
	Bmcr_AutoNegEn = 0x1000,	/*if set ignores Duplex, Speed[01] */
	Bmcr_RstAutoNeg = 0x0200,
	Bmcr_Duplex = 0x0100,
	Bmcr_Speed1 = 0x0040,
	Bmcr_Force10H = 0x0000,
	Bmcr_Force10F = 0x0100,
	Bmcr_Force100H = 0x2000,
	Bmcr_Force100F = 0x2100,
	Bmcr_Force1000H = 0x0040,
	Bmcr_Force1000F = 0x0140,
};

/* auto negotiation advertisement register */
enum anar_bits {
	anar_adv_100F = 0x0100,
	anar_adv_100H = 0x0080,
	anar_adv_10F = 0x0040,
	anar_adv_10H = 0x0020,
	anar_ieee_8023 = 0x0001,
};

/* 1K-base T control register */
enum ktcr_bits {
	ktcr_adv_1000H = 0x0100,
	ktcr_adv_1000F = 0x0200,
};

/* Globals */
static u32 SavedClkRun;
static unsigned int cur_rx;
static unsigned int rx_config;
static unsigned int tx_config;

/* Note: transmit and receive buffers and descriptors must be
   long long word aligned */
static BufferDesc txd __attribute__ ((aligned(8)));
static BufferDesc rxd[NUM_RX_DESC] __attribute__ ((aligned(8)));
static unsigned char txb[TX_BUF_SIZE] __attribute__ ((aligned(8)));
static unsigned char rxb[NUM_RX_DESC * RX_BUF_SIZE]
    __attribute__ ((aligned(8)));

/* Function Prototypes */
static int mdio_read(struct eth_device *dev, int phy_id, int addr);
static void mdio_write(struct eth_device *dev, int phy_id, int addr, int value);
static void mdio_sync(struct eth_device *dev, u32 offset);
static int ns8382x_init(struct eth_device *dev, bd_t * bis);
static void ns8382x_reset(struct eth_device *dev);
static void ns8382x_init_rxfilter(struct eth_device *dev);
static void ns8382x_init_txd(struct eth_device *dev);
static void ns8382x_init_rxd(struct eth_device *dev);
static void ns8382x_set_rx_mode(struct eth_device *dev);
static void ns8382x_check_duplex(struct eth_device *dev);
static int ns8382x_send(struct eth_device *dev, void *packet, int length);
static int ns8382x_poll(struct eth_device *dev);
static void ns8382x_disable(struct eth_device *dev);

static struct pci_device_id supported[] = {
	{PCI_VENDOR_ID_NS, PCI_DEVICE_ID_NS_83820},
	{}
};

#define bus_to_phys(a)	pci_mem_to_phys((pci_dev_t)dev->priv, a)
#define phys_to_bus(a)	pci_phys_to_mem((pci_dev_t)dev->priv, a)

static inline int
INW(struct eth_device *dev, u_long addr)
{
	return le16_to_cpu(*(vu_short *) (addr + dev->iobase));
}

static int
INL(struct eth_device *dev, u_long addr)
{
	return le32_to_cpu(*(vu_long *) (addr + dev->iobase));
}

static inline void
OUTW(struct eth_device *dev, int command, u_long addr)
{
	*(vu_short *) ((addr + dev->iobase)) = cpu_to_le16(command);
}

static inline void
OUTL(struct eth_device *dev, int command, u_long addr)
{
	*(vu_long *) ((addr + dev->iobase)) = cpu_to_le32(command);
}

/* Function: ns8382x_initialize
 * Description: Retrieves the MAC address of the card, and sets up some
 *  globals required by other routines, and initializes the NIC, making it
 *  ready to send and receive packets.
 * Side effects: initializes ns8382xs, ready to receive packets.
 * Returns:   int:          number of cards found
 */

int
ns8382x_initialize(bd_t * bis)
{
	pci_dev_t devno;
	int card_number = 0;
	struct eth_device *dev;
	u32 iobase, status;
	int i, idx = 0;
	u32 phyAddress;
	u32 tmp;
	u32 chip_config;

	while (1) {		/* Find PCI device(s) */
		if ((devno = pci_find_devices(supported, idx++)) < 0)
			break;

		pci_read_config_dword(devno, PCI_BASE_ADDRESS_1, &iobase);
		iobase &= ~0x3;	/* 1: unused and 0:I/O Space Indicator */

		debug("ns8382x: NatSemi dp8382x @ 0x%x\n", iobase);

		pci_write_config_dword(devno, PCI_COMMAND,
				       PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER);

		/* Check if I/O accesses and Bus Mastering are enabled. */
		pci_read_config_dword(devno, PCI_COMMAND, &status);
		if (!(status & PCI_COMMAND_MEMORY)) {
			printf("Error: Can not enable MEM access.\n");
			continue;
		} else if (!(status & PCI_COMMAND_MASTER)) {
			printf("Error: Can not enable Bus Mastering.\n");
			continue;
		}

		dev = (struct eth_device *) malloc(sizeof *dev);
		if (!dev) {
			printf("ns8382x: Can not allocate memory\n");
			break;
		}
		memset(dev, 0, sizeof(*dev));

		sprintf(dev->name, "dp8382x#%d", card_number);
		dev->iobase = bus_to_phys(iobase);
		dev->priv = (void *) devno;
		dev->init = ns8382x_init;
		dev->halt = ns8382x_disable;
		dev->send = ns8382x_send;
		dev->recv = ns8382x_poll;

		/* ns8382x has a non-standard PM control register
		 * in PCI config space.  Some boards apparently need
		 * to be brought to D0 in this manner.  */
		pci_read_config_dword(devno, PCIPM, &tmp);
		if (tmp & (0x03 | 0x100)) {	/* D0 state, disable PME assertion */
			u32 newtmp = tmp & ~(0x03 | 0x100);
			pci_write_config_dword(devno, PCIPM, newtmp);
		}

		/* get MAC address */
		for (i = 0; i < 3; i++) {
			u32 data;
			char *mac = (char *)&dev->enetaddr[i * 2];

			OUTL(dev, i * 2, RxFilterAddr);
			data = INL(dev, RxFilterData);
			*mac++ = data;
			*mac++ = data >> 8;
		}
		/* get PHY address, can't be zero */
		for (phyAddress = 1; phyAddress < 32; phyAddress++) {
			u32 rev, phy1;

			phy1 = mdio_read(dev, phyAddress, PHYIDR1);
			if (phy1 == 0x2000) {	/*check for 83861/91 */
				rev = mdio_read(dev, phyAddress, PHYIDR2);
				if ((rev & ~(0x000f)) == 0x00005c50 ||
				    (rev & ~(0x000f)) == 0x00005c60) {
					debug("phy rev is %x\n", rev);
					debug("phy address is %x\n",
					       phyAddress);
					break;
				}
			}
		}

		/* set phy to autonegotiate && advertise everything */
		mdio_write(dev, phyAddress, KTCR,
			   (ktcr_adv_1000H | ktcr_adv_1000F));
		mdio_write(dev, phyAddress, ANAR,
			   (anar_adv_100F | anar_adv_100H | anar_adv_10H |
			    anar_adv_10F | anar_ieee_8023));
		mdio_write(dev, phyAddress, BMCR, 0x0);	/*restore */
		mdio_write(dev, phyAddress, BMCR,
			   (Bmcr_AutoNegEn | Bmcr_RstAutoNeg));
		/* Reset the chip to erase any previous misconfiguration. */
		OUTL(dev, (ChipReset), ChipCmd);

		chip_config = INL(dev, ChipConfig);
		/* reset the phy */
		OUTL(dev, (chip_config | PhyRst), ChipConfig);
		/* power up and initialize transceiver */
		OUTL(dev, (chip_config & ~(PhyDis)), ChipConfig);

		mdio_sync(dev, EECtrl);

		{
			u32 chpcfg =
			    INL(dev, ChipConfig) ^ SpeedStatus_Polarity;

			debug("%s: Transceiver 10%s %s duplex.\n", dev->name,
			       (chpcfg & GigSpeed) ? "00" : (chpcfg & HundSpeed)
			       ? "0" : "",
			       chpcfg & FullDuplex ? "full" : "half");
			debug("%s: %02x:%02x:%02x:%02x:%02x:%02x\n", dev->name,
			       dev->enetaddr[0], dev->enetaddr[1],
			       dev->enetaddr[2], dev->enetaddr[3],
			       dev->enetaddr[4], dev->enetaddr[5]);
		}

		/* Disable PME:
		 * The PME bit is initialized from the EEPROM contents.
		 * PCI cards probably have PME disabled, but motherboard
		 * implementations may have PME set to enable WakeOnLan.
		 * With PME set the chip will scan incoming packets but
		 * nothing will be written to memory. */
		SavedClkRun = INL(dev, ClkRun);
		OUTL(dev, SavedClkRun & ~0x100, ClkRun);

		eth_register(dev);

		card_number++;

		pci_write_config_byte(devno, PCI_LATENCY_TIMER, 0x60);

		udelay(10 * 1000);
	}
	return card_number;
}

/*  MII transceiver control section.
	Read and write MII registers using software-generated serial MDIO
	protocol.  See the MII specifications or DP83840A data sheet for details.

	The maximum data clock rate is 2.5 MHz.  To meet minimum timing we
	must flush writes to the PCI bus with a PCI read. */
#define mdio_delay(mdio_addr) INL(dev, mdio_addr)

#define MDIO_EnbIn  (0)
#define MDIO_WRITE0 (MDIO_EnbOutput)
#define MDIO_WRITE1 (MDIO_Data | MDIO_EnbOutput)

/* Generate the preamble required for initial synchronization and
   a few older transceivers. */
static void
mdio_sync(struct eth_device *dev, u32 offset)
{
	int bits = 32;

	/* Establish sync by sending at least 32 logic ones. */
	while (--bits >= 0) {
		OUTL(dev, MDIO_WRITE1, offset);
		mdio_delay(offset);
		OUTL(dev, MDIO_WRITE1 | MDIO_ShiftClk, offset);
		mdio_delay(offset);
	}
}

static int
mdio_read(struct eth_device *dev, int phy_id, int addr)
{
	int mii_cmd = (0xf6 << 10) | (phy_id << 5) | addr;
	int i, retval = 0;

	/* Shift the read command bits out. */
	for (i = 15; i >= 0; i--) {
		int dataval = (mii_cmd & (1 << i)) ? MDIO_WRITE1 : MDIO_WRITE0;

		OUTL(dev, dataval, EECtrl);
		mdio_delay(EECtrl);
		OUTL(dev, dataval | MDIO_ShiftClk, EECtrl);
		mdio_delay(EECtrl);
	}
	/* Read the two transition, 16 data, and wire-idle bits. */
	for (i = 19; i > 0; i--) {
		OUTL(dev, MDIO_EnbIn, EECtrl);
		mdio_delay(EECtrl);
		retval =
		    (retval << 1) | ((INL(dev, EECtrl) & MDIO_Data) ? 1 : 0);
		OUTL(dev, MDIO_EnbIn | MDIO_ShiftClk, EECtrl);
		mdio_delay(EECtrl);
	}
	return (retval >> 1) & 0xffff;
}

static void
mdio_write(struct eth_device *dev, int phy_id, int addr, int value)
{
	int mii_cmd = (0x5002 << 16) | (phy_id << 23) | (addr << 18) | value;
	int i;

	/* Shift the command bits out. */
	for (i = 31; i >= 0; i--) {
		int dataval = (mii_cmd & (1 << i)) ? MDIO_WRITE1 : MDIO_WRITE0;

		OUTL(dev, dataval, EECtrl);
		mdio_delay(EECtrl);
		OUTL(dev, dataval | MDIO_ShiftClk, EECtrl);
		mdio_delay(EECtrl);
	}
	/* Clear out extra bits. */
	for (i = 2; i > 0; i--) {
		OUTL(dev, MDIO_EnbIn, EECtrl);
		mdio_delay(EECtrl);
		OUTL(dev, MDIO_EnbIn | MDIO_ShiftClk, EECtrl);
		mdio_delay(EECtrl);
	}
	return;
}

/* Function: ns8382x_init
 * Description: resets the ethernet controller chip and configures
 *    registers and data structures required for sending and receiving packets.
 * Arguments: struct eth_device *dev:       NIC data structure
 * returns:	int.
 */

static int
ns8382x_init(struct eth_device *dev, bd_t * bis)
{
	u32 config;

	ns8382x_reset(dev);

	/* Disable PME:
	 * The PME bit is initialized from the EEPROM contents.
	 * PCI cards probably have PME disabled, but motherboard
	 * implementations may have PME set to enable WakeOnLan.
	 * With PME set the chip will scan incoming packets but
	 * nothing will be written to memory. */
	OUTL(dev, SavedClkRun & ~0x100, ClkRun);

	ns8382x_init_rxfilter(dev);
	ns8382x_init_txd(dev);
	ns8382x_init_rxd(dev);

	/*set up ChipConfig */
	config = INL(dev, ChipConfig);
	/*turn off 64 bit ops && Ten-bit interface
	 * && big-endian mode && extended status */
	config &= ~(TBIEn | Mode1000 | T64En | D64En | M64En | BEMode | PhyDis | ExtStEn);
	OUTL(dev, config, ChipConfig);

	/* Configure the PCI bus bursts and FIFO thresholds. */
	tx_config = TxCarrierIgn | TxHeartIgn | TxAutoPad
	    | TxCollRetry | TxMxdma_1024 | (0x1002);
	rx_config = RxMxdma_1024 | 0x20;

	debug("%s: Setting TxConfig Register %#08X\n", dev->name, tx_config);
	debug("%s: Setting RxConfig Register %#08X\n", dev->name, rx_config);

	OUTL(dev, tx_config, TxConfig);
	OUTL(dev, rx_config, RxConfig);

	/*turn off priority queueing */
	OUTL(dev, 0x0, PriQueue);

	ns8382x_check_duplex(dev);
	ns8382x_set_rx_mode(dev);

	OUTL(dev, (RxOn | TxOn), ChipCmd);
	return 1;
}

/* Function: ns8382x_reset
 * Description: soft resets the controller chip
 * Arguments: struct eth_device *dev:          NIC data structure
 * Returns:   void.
 */
static void
ns8382x_reset(struct eth_device *dev)
{
	OUTL(dev, ChipReset, ChipCmd);
	while (INL(dev, ChipCmd))
		/*wait until done */ ;
	OUTL(dev, 0, IntrMask);
	OUTL(dev, 0, IntrEnable);
}

/* Function: ns8382x_init_rxfilter
 * Description: sets receive filter address to our MAC address
 * Arguments: struct eth_device *dev:          NIC data structure
 * returns:   void.
 */

static void
ns8382x_init_rxfilter(struct eth_device *dev)
{
	int i;

	for (i = 0; i < ETH_ALEN; i += 2) {
		OUTL(dev, i, RxFilterAddr);
		OUTW(dev, dev->enetaddr[i] + (dev->enetaddr[i + 1] << 8),
		     RxFilterData);
	}
}

/* Function: ns8382x_init_txd
 * Description: initializes the Tx descriptor
 * Arguments: struct eth_device *dev:          NIC data structure
 * returns:   void.
 */

static void
ns8382x_init_txd(struct eth_device *dev)
{
	txd.link = (u32) 0;
	txd.bufptr = cpu_to_le32((u32) & txb[0]);
	txd.cmdsts = (u32) 0;
	txd.extsts = (u32) 0;

	OUTL(dev, 0x0, TxRingPtrHi);
	OUTL(dev, phys_to_bus((u32)&txd), TxRingPtr);

	debug("ns8382x_init_txd: TX descriptor register loaded with: %#08X (&txd: %p)\n",
	       INL(dev, TxRingPtr), &txd);
}

/* Function: ns8382x_init_rxd
 * Description: initializes the Rx descriptor ring
 * Arguments: struct eth_device *dev:          NIC data structure
 * Returns:   void.
 */

static void
ns8382x_init_rxd(struct eth_device *dev)
{
	int i;

	OUTL(dev, 0x0, RxRingPtrHi);

	cur_rx = 0;
	for (i = 0; i < NUM_RX_DESC; i++) {
		rxd[i].link =
		    cpu_to_le32((i + 1 <
				 NUM_RX_DESC) ? (u32) & rxd[i +
							    1] : (u32) &
				rxd[0]);
		rxd[i].extsts = cpu_to_le32((u32) 0x0);
		rxd[i].cmdsts = cpu_to_le32((u32) RX_BUF_SIZE);
		rxd[i].bufptr = cpu_to_le32((u32) & rxb[i * RX_BUF_SIZE]);

		debug
		    ("ns8382x_init_rxd: rxd[%d]=%p link=%X cmdsts=%X bufptr=%X\n",
		     i, &rxd[i], le32_to_cpu(rxd[i].link),
		     le32_to_cpu(rxd[i].cmdsts), le32_to_cpu(rxd[i].bufptr));
	}
	OUTL(dev, phys_to_bus((u32) & rxd), RxRingPtr);

	debug("ns8382x_init_rxd: RX descriptor register loaded with: %X\n",
	       INL(dev, RxRingPtr));
}

/* Function: ns8382x_set_rx_mode
 * Description:
 *    sets the receive mode to accept all broadcast packets and packets
 *    with our MAC address, and reject all multicast packets.
 * Arguments: struct eth_device *dev:          NIC data structure
 * Returns:   void.
 */

static void
ns8382x_set_rx_mode(struct eth_device *dev)
{
	u32 rx_mode = 0x0;
	/*spec says RxFilterEnable has to be 0 for rest of
	 * this stuff to be properly configured. Linux driver
	 * seems to support this*/
/*	OUTL(dev, rx_mode, RxFilterAddr);*/
	rx_mode = (RxFilterEnable | AcceptAllBroadcast | AcceptPerfectMatch);
	OUTL(dev, rx_mode, RxFilterAddr);
	printf("ns8382x_set_rx_mode: set to %X\n", rx_mode);
	/*now we turn RxFilterEnable back on */
	/*rx_mode |= RxFilterEnable;
	OUTL(dev, rx_mode, RxFilterAddr);*/
}

static void
ns8382x_check_duplex(struct eth_device *dev)
{
	int gig = 0;
	int hun = 0;
	int duplex = 0;
	int config = (INL(dev, ChipConfig) ^ SpeedStatus_Polarity);

	duplex = (config & FullDuplex) ? 1 : 0;
	gig = (config & GigSpeed) ? 1 : 0;
	hun = (config & HundSpeed) ? 1 : 0;

	debug("%s: Setting 10%s %s-duplex based on negotiated link"
	       " capability.\n", dev->name, (gig) ? "00" : (hun) ? "0" : "",
	       duplex ? "full" : "half");

	if (duplex) {
		rx_config |= RxAcceptTx;
		tx_config |= (TxCarrierIgn | TxHeartIgn);
	} else {
		rx_config &= ~RxAcceptTx;
		tx_config &= ~(TxCarrierIgn | TxHeartIgn);
	}

	debug("%s: Resetting TxConfig Register %#08X\n", dev->name, tx_config);
	debug("%s: Resetting RxConfig Register %#08X\n", dev->name, rx_config);

	OUTL(dev, tx_config, TxConfig);
	OUTL(dev, rx_config, RxConfig);

	/*if speed is 10 or 100, remove MODE1000,
	 * if it's 1000, then set it */
	config = INL(dev, ChipConfig);
	if (gig)
		config |= Mode1000;
	else
		config &= ~Mode1000;

	debug("%s: %setting Mode1000\n", dev->name, (gig) ? "S" : "Uns");

	OUTL(dev, config, ChipConfig);
}

/* Function: ns8382x_send
 * Description: transmits a packet and waits for completion or timeout.
 * Returns:   void.  */
static int ns8382x_send(struct eth_device *dev, void *packet, int length)
{
	u32 i, status = 0;
	vu_long tx_stat = 0;

	/* Stop the transmitter */
	OUTL(dev, TxOff, ChipCmd);

	debug("ns8382x_send: sending %d bytes\n", (int)length);

	/* set the transmit buffer descriptor and enable Transmit State Machine */
	txd.link = cpu_to_le32(0x0);
	txd.bufptr = cpu_to_le32(phys_to_bus((u32)packet));
	txd.extsts = cpu_to_le32(0x0);
	txd.cmdsts = cpu_to_le32(DescOwn | length);

	/* load Transmit Descriptor Register */
	OUTL(dev, phys_to_bus((u32) & txd), TxRingPtr);

	debug("ns8382x_send: TX descriptor register loaded with: %#08X\n",
	       INL(dev, TxRingPtr));
	debug("\ttxd.link:%X\tbufp:%X\texsts:%X\tcmdsts:%X\n",
	       le32_to_cpu(txd.link), le32_to_cpu(txd.bufptr),
	       le32_to_cpu(txd.extsts), le32_to_cpu(txd.cmdsts));

	/* restart the transmitter */
	OUTL(dev, TxOn, ChipCmd);

	for (i = 0; (tx_stat = le32_to_cpu(txd.cmdsts)) & DescOwn; i++) {
		if (i >= TOUT_LOOP) {
			printf ("%s: tx error buffer not ready: txd.cmdsts %#lX\n",
			     dev->name, tx_stat);
			goto Done;
		}
	}

	if (!(tx_stat & DescPktOK)) {
		printf("ns8382x_send: Transmit error, Tx status %lX.\n", tx_stat);
		goto Done;
	}

	debug("ns8382x_send: tx_stat: %#08lX\n", tx_stat);

	status = 1;
Done:
	return status;
}

/* Function: ns8382x_poll
 * Description: checks for a received packet and returns it if found.
 * Arguments: struct eth_device *dev:          NIC data structure
 * Returns:   1 if    packet was received.
 *            0 if no packet was received.
 * Side effects:
 *            Returns (copies) the packet to the array dev->packet.
 *            Returns the length of the packet.
 */

static int
ns8382x_poll(struct eth_device *dev)
{
	int retstat = 0;
	int length = 0;
	vu_long rx_status = le32_to_cpu(rxd[cur_rx].cmdsts);

	if (!(rx_status & (u32) DescOwn))
		return retstat;

	debug("ns8382x_poll: got a packet: cur_rx:%u, status:%lx\n",
	       cur_rx, rx_status);

	length = (rx_status & DSIZE) - CRC_SIZE;

	if ((rx_status & (DescMore | DescPktOK | DescRxLong)) != DescPktOK) {
		/* corrupted packet received */
		printf("ns8382x_poll: Corrupted packet, status:%lx\n",
		       rx_status);
		retstat = 0;
	} else {
		/* give packet to higher level routine */
		net_process_received_packet((rxb + cur_rx * RX_BUF_SIZE),
					    length);
		retstat = 1;
	}

	/* return the descriptor and buffer to receive ring */
	rxd[cur_rx].cmdsts = cpu_to_le32(RX_BUF_SIZE);
	rxd[cur_rx].bufptr = cpu_to_le32((u32) & rxb[cur_rx * RX_BUF_SIZE]);

	if (++cur_rx == NUM_RX_DESC)
		cur_rx = 0;

	/* re-enable the potentially idle receive state machine */
	OUTL(dev, RxOn, ChipCmd);

	return retstat;
}

/* Function: ns8382x_disable
 * Description: Turns off interrupts and stops Tx and Rx engines
 * Arguments: struct eth_device *dev:          NIC data structure
 * Returns:   void.
 */

static void
ns8382x_disable(struct eth_device *dev)
{
	/* Disable interrupts using the mask. */
	OUTL(dev, 0, IntrMask);
	OUTL(dev, 0, IntrEnable);

	/* Stop the chip's Tx and Rx processes. */
	OUTL(dev, (RxOff | TxOff), ChipCmd);

	/* Restore PME enable bit */
	OUTL(dev, SavedClkRun, ClkRun);
}
