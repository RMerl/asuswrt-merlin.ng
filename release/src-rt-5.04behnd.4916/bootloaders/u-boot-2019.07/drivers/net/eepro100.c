// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <common.h>
#include <malloc.h>
#include <net.h>
#include <netdev.h>
#include <asm/io.h>
#include <pci.h>
#include <miiphy.h>

#undef DEBUG

	/* Ethernet chip registers.
	 */
#define SCBStatus		0	/* Rx/Command Unit Status *Word* */
#define SCBIntAckByte		1	/* Rx/Command Unit STAT/ACK byte */
#define SCBCmd			2	/* Rx/Command Unit Command *Word* */
#define SCBIntrCtlByte		3	/* Rx/Command Unit Intr.Control Byte */
#define SCBPointer		4	/* General purpose pointer. */
#define SCBPort			8	/* Misc. commands and operands. */
#define SCBflash		12	/* Flash memory control. */
#define SCBeeprom		14	/* EEPROM memory control. */
#define SCBCtrlMDI		16	/* MDI interface control. */
#define SCBEarlyRx		20	/* Early receive byte count. */
#define SCBGenControl		28	/* 82559 General Control Register */
#define SCBGenStatus		29	/* 82559 General Status register */

	/* 82559 SCB status word defnitions
	 */
#define SCB_STATUS_CX		0x8000	/* CU finished command (transmit) */
#define SCB_STATUS_FR		0x4000	/* frame received */
#define SCB_STATUS_CNA		0x2000	/* CU left active state */
#define SCB_STATUS_RNR		0x1000	/* receiver left ready state */
#define SCB_STATUS_MDI		0x0800	/* MDI read/write cycle done */
#define SCB_STATUS_SWI		0x0400	/* software generated interrupt */
#define SCB_STATUS_FCP		0x0100	/* flow control pause interrupt */

#define SCB_INTACK_MASK		0xFD00	/* all the above */

#define SCB_INTACK_TX		(SCB_STATUS_CX | SCB_STATUS_CNA)
#define SCB_INTACK_RX		(SCB_STATUS_FR | SCB_STATUS_RNR)

	/* System control block commands
	 */
/* CU Commands */
#define CU_NOP			0x0000
#define CU_START		0x0010
#define CU_RESUME		0x0020
#define CU_STATSADDR		0x0040	/* Load Dump Statistics ctrs addr */
#define CU_SHOWSTATS		0x0050	/* Dump statistics counters. */
#define CU_ADDR_LOAD		0x0060	/* Base address to add to CU commands */
#define CU_DUMPSTATS		0x0070	/* Dump then reset stats counters. */

/* RUC Commands */
#define RUC_NOP			0x0000
#define RUC_START		0x0001
#define RUC_RESUME		0x0002
#define RUC_ABORT		0x0004
#define RUC_ADDR_LOAD		0x0006	/* (seems not to clear on acceptance) */
#define RUC_RESUMENR		0x0007

#define CU_CMD_MASK		0x00f0
#define RU_CMD_MASK		0x0007

#define SCB_M			0x0100	/* 0 = enable interrupt, 1 = disable */
#define SCB_SWI			0x0200	/* 1 - cause device to interrupt */

#define CU_STATUS_MASK		0x00C0
#define RU_STATUS_MASK		0x003C

#define RU_STATUS_IDLE		(0<<2)
#define RU_STATUS_SUS		(1<<2)
#define RU_STATUS_NORES		(2<<2)
#define RU_STATUS_READY		(4<<2)
#define RU_STATUS_NO_RBDS_SUS	((1<<2)|(8<<2))
#define RU_STATUS_NO_RBDS_NORES ((2<<2)|(8<<2))
#define RU_STATUS_NO_RBDS_READY ((4<<2)|(8<<2))

	/* 82559 Port interface commands.
	 */
#define I82559_RESET		0x00000000	/* Software reset */
#define I82559_SELFTEST		0x00000001	/* 82559 Selftest command */
#define I82559_SELECTIVE_RESET	0x00000002
#define I82559_DUMP		0x00000003
#define I82559_DUMP_WAKEUP	0x00000007

	/* 82559 Eeprom interface.
	 */
#define EE_SHIFT_CLK		0x01	/* EEPROM shift clock. */
#define EE_CS			0x02	/* EEPROM chip select. */
#define EE_DATA_WRITE		0x04	/* EEPROM chip data in. */
#define EE_WRITE_0		0x01
#define EE_WRITE_1		0x05
#define EE_DATA_READ		0x08	/* EEPROM chip data out. */
#define EE_ENB			(0x4800 | EE_CS)
#define EE_CMD_BITS		3
#define EE_DATA_BITS		16

	/* The EEPROM commands include the alway-set leading bit.
	 */
#define EE_EWENB_CMD		(4 << addr_len)
#define EE_WRITE_CMD		(5 << addr_len)
#define EE_READ_CMD		(6 << addr_len)
#define EE_ERASE_CMD		(7 << addr_len)

	/* Receive frame descriptors.
	 */
struct RxFD {
	volatile u16 status;
	volatile u16 control;
	volatile u32 link;		/* struct RxFD * */
	volatile u32 rx_buf_addr;	/* void * */
	volatile u32 count;

	volatile u8 data[PKTSIZE_ALIGN];
};

#define RFD_STATUS_C		0x8000	/* completion of received frame */
#define RFD_STATUS_OK		0x2000	/* frame received with no errors */

#define RFD_CONTROL_EL		0x8000	/* 1=last RFD in RFA */
#define RFD_CONTROL_S		0x4000	/* 1=suspend RU after receiving frame */
#define RFD_CONTROL_H		0x0010	/* 1=RFD is a header RFD */
#define RFD_CONTROL_SF		0x0008	/* 0=simplified, 1=flexible mode */

#define RFD_COUNT_MASK		0x3fff
#define RFD_COUNT_F		0x4000
#define RFD_COUNT_EOF		0x8000

#define RFD_RX_CRC		0x0800	/* crc error */
#define RFD_RX_ALIGNMENT	0x0400	/* alignment error */
#define RFD_RX_RESOURCE		0x0200	/* out of space, no resources */
#define RFD_RX_DMA_OVER		0x0100	/* DMA overrun */
#define RFD_RX_SHORT		0x0080	/* short frame error */
#define RFD_RX_LENGTH		0x0020
#define RFD_RX_ERROR		0x0010	/* receive error */
#define RFD_RX_NO_ADR_MATCH	0x0004	/* no address match */
#define RFD_RX_IA_MATCH		0x0002	/* individual address does not match */
#define RFD_RX_TCO		0x0001	/* TCO indication */

	/* Transmit frame descriptors
	 */
struct TxFD {				/* Transmit frame descriptor set. */
	volatile u16 status;
	volatile u16 command;
	volatile u32 link;		/* void * */
	volatile u32 tx_desc_addr;	/* Always points to the tx_buf_addr element. */
	volatile s32 count;

	volatile u32 tx_buf_addr0;	/* void *, frame to be transmitted.  */
	volatile s32 tx_buf_size0;	/* Length of Tx frame. */
	volatile u32 tx_buf_addr1;	/* void *, frame to be transmitted.  */
	volatile s32 tx_buf_size1;	/* Length of Tx frame. */
};

#define TxCB_CMD_TRANSMIT	0x0004	/* transmit command */
#define TxCB_CMD_SF		0x0008	/* 0=simplified, 1=flexible mode */
#define TxCB_CMD_NC		0x0010	/* 0=CRC insert by controller */
#define TxCB_CMD_I		0x2000	/* generate interrupt on completion */
#define TxCB_CMD_S		0x4000	/* suspend on completion */
#define TxCB_CMD_EL		0x8000	/* last command block in CBL */

#define TxCB_COUNT_MASK		0x3fff
#define TxCB_COUNT_EOF		0x8000

	/* The Speedo3 Rx and Tx frame/buffer descriptors.
	 */
struct descriptor {			/* A generic descriptor. */
	volatile u16 status;
	volatile u16 command;
	volatile u32 link;		/* struct descriptor *	*/

	unsigned char params[0];
};

#define CONFIG_SYS_CMD_EL		0x8000
#define CONFIG_SYS_CMD_SUSPEND		0x4000
#define CONFIG_SYS_CMD_INT		0x2000
#define CONFIG_SYS_CMD_IAS		0x0001	/* individual address setup */
#define CONFIG_SYS_CMD_CONFIGURE	0x0002	/* configure */

#define CONFIG_SYS_STATUS_C		0x8000
#define CONFIG_SYS_STATUS_OK		0x2000

	/* Misc.
	 */
#define NUM_RX_DESC		PKTBUFSRX
#define NUM_TX_DESC		1	/* Number of TX descriptors   */

#define TOUT_LOOP		1000000

static struct RxFD rx_ring[NUM_RX_DESC];	/* RX descriptor ring	      */
static struct TxFD tx_ring[NUM_TX_DESC];	/* TX descriptor ring	      */
static int rx_next;			/* RX descriptor ring pointer */
static int tx_next;			/* TX descriptor ring pointer */
static int tx_threshold;

/*
 * The parameters for a CmdConfigure operation.
 * There are so many options that it would be difficult to document
 * each bit. We mostly use the default or recommended settings.
 */
static const char i82558_config_cmd[] = {
	22, 0x08, 0, 1, 0, 0, 0x22, 0x03, 1,	/* 1=Use MII  0=Use AUI */
	0, 0x2E, 0, 0x60, 0x08, 0x88,
	0x68, 0, 0x40, 0xf2, 0x84,		/* Disable FC */
	0x31, 0x05,
};

static void init_rx_ring (struct eth_device *dev);
static void purge_tx_ring (struct eth_device *dev);

static void read_hw_addr (struct eth_device *dev, bd_t * bis);

static int eepro100_init (struct eth_device *dev, bd_t * bis);
static int eepro100_send(struct eth_device *dev, void *packet, int length);
static int eepro100_recv (struct eth_device *dev);
static void eepro100_halt (struct eth_device *dev);

#if defined(CONFIG_E500)
#define bus_to_phys(a) (a)
#define phys_to_bus(a) (a)
#else
#define bus_to_phys(a)	pci_mem_to_phys((pci_dev_t)dev->priv, a)
#define phys_to_bus(a)	pci_phys_to_mem((pci_dev_t)dev->priv, a)
#endif

static inline int INW (struct eth_device *dev, u_long addr)
{
	return le16_to_cpu(*(volatile u16 *)(addr + (u_long)dev->iobase));
}

static inline void OUTW (struct eth_device *dev, int command, u_long addr)
{
	*(volatile u16 *)((addr + (u_long)dev->iobase)) = cpu_to_le16(command);
}

static inline void OUTL (struct eth_device *dev, int command, u_long addr)
{
	*(volatile u32 *)((addr + (u_long)dev->iobase)) = cpu_to_le32(command);
}

#if defined(CONFIG_MII) || defined(CONFIG_CMD_MII)
static inline int INL (struct eth_device *dev, u_long addr)
{
	return le32_to_cpu(*(volatile u32 *)(addr + (u_long)dev->iobase));
}

static int get_phyreg (struct eth_device *dev, unsigned char addr,
		unsigned char reg, unsigned short *value)
{
	int cmd;
	int timeout = 50;

	/* read requested data */
	cmd = (2 << 26) | ((addr & 0x1f) << 21) | ((reg & 0x1f) << 16);
	OUTL (dev, cmd, SCBCtrlMDI);

	do {
		udelay(1000);
		cmd = INL (dev, SCBCtrlMDI);
	} while (!(cmd & (1 << 28)) && (--timeout));

	if (timeout == 0)
		return -1;

	*value = (unsigned short) (cmd & 0xffff);

	return 0;
}

static int set_phyreg (struct eth_device *dev, unsigned char addr,
		unsigned char reg, unsigned short value)
{
	int cmd;
	int timeout = 50;

	/* write requested data */
	cmd = (1 << 26) | ((addr & 0x1f) << 21) | ((reg & 0x1f) << 16);
	OUTL (dev, cmd | value, SCBCtrlMDI);

	while (!(INL (dev, SCBCtrlMDI) & (1 << 28)) && (--timeout))
		udelay(1000);

	if (timeout == 0)
		return -1;

	return 0;
}

/* Check if given phyaddr is valid, i.e. there is a PHY connected.
 * Do this by checking model value field from ID2 register.
 */
static struct eth_device* verify_phyaddr (const char *devname,
						unsigned char addr)
{
	struct eth_device *dev;
	unsigned short value;
	unsigned char model;

	dev = eth_get_dev_by_name(devname);
	if (dev == NULL) {
		printf("%s: no such device\n", devname);
		return NULL;
	}

	/* read id2 register */
	if (get_phyreg(dev, addr, MII_PHYSID2, &value) != 0) {
		printf("%s: mii read timeout!\n", devname);
		return NULL;
	}

	/* get model */
	model = (unsigned char)((value >> 4) & 0x003f);

	if (model == 0) {
		printf("%s: no PHY at address %d\n", devname, addr);
		return NULL;
	}

	return dev;
}

static int eepro100_miiphy_read(struct mii_dev *bus, int addr, int devad,
				int reg)
{
	unsigned short value = 0;
	struct eth_device *dev;

	dev = verify_phyaddr(bus->name, addr);
	if (dev == NULL)
		return -1;

	if (get_phyreg(dev, addr, reg, &value) != 0) {
		printf("%s: mii read timeout!\n", bus->name);
		return -1;
	}

	return value;
}

static int eepro100_miiphy_write(struct mii_dev *bus, int addr, int devad,
				 int reg, u16 value)
{
	struct eth_device *dev;

	dev = verify_phyaddr(bus->name, addr);
	if (dev == NULL)
		return -1;

	if (set_phyreg(dev, addr, reg, value) != 0) {
		printf("%s: mii write timeout!\n", bus->name);
		return -1;
	}

	return 0;
}

#endif

/* Wait for the chip get the command.
*/
static int wait_for_eepro100 (struct eth_device *dev)
{
	int i;

	for (i = 0; INW (dev, SCBCmd) & (CU_CMD_MASK | RU_CMD_MASK); i++) {
		if (i >= TOUT_LOOP) {
			return 0;
		}
	}

	return 1;
}

static struct pci_device_id supported[] = {
	{PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82557},
	{PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82559},
	{PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82559ER},
	{}
};

int eepro100_initialize (bd_t * bis)
{
	pci_dev_t devno;
	int card_number = 0;
	struct eth_device *dev;
	u32 iobase, status;
	int idx = 0;

	while (1) {
		/* Find PCI device
		 */
		if ((devno = pci_find_devices (supported, idx++)) < 0) {
			break;
		}

		pci_read_config_dword (devno, PCI_BASE_ADDRESS_0, &iobase);
		iobase &= ~0xf;

#ifdef DEBUG
		printf ("eepro100: Intel i82559 PCI EtherExpressPro @0x%x\n",
				iobase);
#endif

		pci_write_config_dword (devno,
					PCI_COMMAND,
					PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER);

		/* Check if I/O accesses and Bus Mastering are enabled.
		 */
		pci_read_config_dword (devno, PCI_COMMAND, &status);
		if (!(status & PCI_COMMAND_MEMORY)) {
			printf ("Error: Can not enable MEM access.\n");
			continue;
		}

		if (!(status & PCI_COMMAND_MASTER)) {
			printf ("Error: Can not enable Bus Mastering.\n");
			continue;
		}

		dev = (struct eth_device *) malloc (sizeof *dev);
		if (!dev) {
			printf("eepro100: Can not allocate memory\n");
			break;
		}
		memset(dev, 0, sizeof(*dev));

		sprintf (dev->name, "i82559#%d", card_number);
		dev->priv = (void *) devno; /* this have to come before bus_to_phys() */
		dev->iobase = bus_to_phys (iobase);
		dev->init = eepro100_init;
		dev->halt = eepro100_halt;
		dev->send = eepro100_send;
		dev->recv = eepro100_recv;

		eth_register (dev);

#if defined (CONFIG_MII) || defined(CONFIG_CMD_MII)
		/* register mii command access routines */
		int retval;
		struct mii_dev *mdiodev = mdio_alloc();
		if (!mdiodev)
			return -ENOMEM;
		strncpy(mdiodev->name, dev->name, MDIO_NAME_LEN);
		mdiodev->read = eepro100_miiphy_read;
		mdiodev->write = eepro100_miiphy_write;

		retval = mdio_register(mdiodev);
		if (retval < 0)
			return retval;
#endif

		card_number++;

		/* Set the latency timer for value.
		 */
		pci_write_config_byte (devno, PCI_LATENCY_TIMER, 0x20);

		udelay (10 * 1000);

		read_hw_addr (dev, bis);
	}

	return card_number;
}


static int eepro100_init (struct eth_device *dev, bd_t * bis)
{
	int i, status = -1;
	int tx_cur;
	struct descriptor *ias_cmd, *cfg_cmd;

	/* Reset the ethernet controller
	 */
	OUTL (dev, I82559_SELECTIVE_RESET, SCBPort);
	udelay (20);

	OUTL (dev, I82559_RESET, SCBPort);
	udelay (20);

	if (!wait_for_eepro100 (dev)) {
		printf ("Error: Can not reset ethernet controller.\n");
		goto Done;
	}
	OUTL (dev, 0, SCBPointer);
	OUTW (dev, SCB_M | RUC_ADDR_LOAD, SCBCmd);

	if (!wait_for_eepro100 (dev)) {
		printf ("Error: Can not reset ethernet controller.\n");
		goto Done;
	}
	OUTL (dev, 0, SCBPointer);
	OUTW (dev, SCB_M | CU_ADDR_LOAD, SCBCmd);

	/* Initialize Rx and Tx rings.
	 */
	init_rx_ring (dev);
	purge_tx_ring (dev);

	/* Tell the adapter where the RX ring is located.
	 */
	if (!wait_for_eepro100 (dev)) {
		printf ("Error: Can not reset ethernet controller.\n");
		goto Done;
	}

	OUTL (dev, phys_to_bus ((u32) & rx_ring[rx_next]), SCBPointer);
	OUTW (dev, SCB_M | RUC_START, SCBCmd);

	/* Send the Configure frame */
	tx_cur = tx_next;
	tx_next = ((tx_next + 1) % NUM_TX_DESC);

	cfg_cmd = (struct descriptor *) &tx_ring[tx_cur];
	cfg_cmd->command = cpu_to_le16 ((CONFIG_SYS_CMD_SUSPEND | CONFIG_SYS_CMD_CONFIGURE));
	cfg_cmd->status = 0;
	cfg_cmd->link = cpu_to_le32 (phys_to_bus ((u32) & tx_ring[tx_next]));

	memcpy (cfg_cmd->params, i82558_config_cmd,
			sizeof (i82558_config_cmd));

	if (!wait_for_eepro100 (dev)) {
		printf ("Error---CONFIG_SYS_CMD_CONFIGURE: Can not reset ethernet controller.\n");
		goto Done;
	}

	OUTL (dev, phys_to_bus ((u32) & tx_ring[tx_cur]), SCBPointer);
	OUTW (dev, SCB_M | CU_START, SCBCmd);

	for (i = 0;
	     !(le16_to_cpu (tx_ring[tx_cur].status) & CONFIG_SYS_STATUS_C);
	     i++) {
		if (i >= TOUT_LOOP) {
			printf ("%s: Tx error buffer not ready\n", dev->name);
			goto Done;
		}
	}

	if (!(le16_to_cpu (tx_ring[tx_cur].status) & CONFIG_SYS_STATUS_OK)) {
		printf ("TX error status = 0x%08X\n",
			le16_to_cpu (tx_ring[tx_cur].status));
		goto Done;
	}

	/* Send the Individual Address Setup frame
	 */
	tx_cur = tx_next;
	tx_next = ((tx_next + 1) % NUM_TX_DESC);

	ias_cmd = (struct descriptor *) &tx_ring[tx_cur];
	ias_cmd->command = cpu_to_le16 ((CONFIG_SYS_CMD_SUSPEND | CONFIG_SYS_CMD_IAS));
	ias_cmd->status = 0;
	ias_cmd->link = cpu_to_le32 (phys_to_bus ((u32) & tx_ring[tx_next]));

	memcpy (ias_cmd->params, dev->enetaddr, 6);

	/* Tell the adapter where the TX ring is located.
	 */
	if (!wait_for_eepro100 (dev)) {
		printf ("Error: Can not reset ethernet controller.\n");
		goto Done;
	}

	OUTL (dev, phys_to_bus ((u32) & tx_ring[tx_cur]), SCBPointer);
	OUTW (dev, SCB_M | CU_START, SCBCmd);

	for (i = 0; !(le16_to_cpu (tx_ring[tx_cur].status) & CONFIG_SYS_STATUS_C);
		 i++) {
		if (i >= TOUT_LOOP) {
			printf ("%s: Tx error buffer not ready\n",
				dev->name);
			goto Done;
		}
	}

	if (!(le16_to_cpu (tx_ring[tx_cur].status) & CONFIG_SYS_STATUS_OK)) {
		printf ("TX error status = 0x%08X\n",
			le16_to_cpu (tx_ring[tx_cur].status));
		goto Done;
	}

	status = 0;

  Done:
	return status;
}

static int eepro100_send(struct eth_device *dev, void *packet, int length)
{
	int i, status = -1;
	int tx_cur;

	if (length <= 0) {
		printf ("%s: bad packet size: %d\n", dev->name, length);
		goto Done;
	}

	tx_cur = tx_next;
	tx_next = (tx_next + 1) % NUM_TX_DESC;

	tx_ring[tx_cur].command = cpu_to_le16 ( TxCB_CMD_TRANSMIT |
						TxCB_CMD_SF	|
						TxCB_CMD_S	|
						TxCB_CMD_EL );
	tx_ring[tx_cur].status = 0;
	tx_ring[tx_cur].count = cpu_to_le32 (tx_threshold);
	tx_ring[tx_cur].link =
		cpu_to_le32 (phys_to_bus ((u32) & tx_ring[tx_next]));
	tx_ring[tx_cur].tx_desc_addr =
		cpu_to_le32 (phys_to_bus ((u32) & tx_ring[tx_cur].tx_buf_addr0));
	tx_ring[tx_cur].tx_buf_addr0 =
		cpu_to_le32 (phys_to_bus ((u_long) packet));
	tx_ring[tx_cur].tx_buf_size0 = cpu_to_le32 (length);

	if (!wait_for_eepro100 (dev)) {
		printf ("%s: Tx error ethernet controller not ready.\n",
				dev->name);
		goto Done;
	}

	/* Send the packet.
	 */
	OUTL (dev, phys_to_bus ((u32) & tx_ring[tx_cur]), SCBPointer);
	OUTW (dev, SCB_M | CU_START, SCBCmd);

	for (i = 0; !(le16_to_cpu (tx_ring[tx_cur].status) & CONFIG_SYS_STATUS_C);
		 i++) {
		if (i >= TOUT_LOOP) {
			printf ("%s: Tx error buffer not ready\n", dev->name);
			goto Done;
		}
	}

	if (!(le16_to_cpu (tx_ring[tx_cur].status) & CONFIG_SYS_STATUS_OK)) {
		printf ("TX error status = 0x%08X\n",
			le16_to_cpu (tx_ring[tx_cur].status));
		goto Done;
	}

	status = length;

  Done:
	return status;
}

static int eepro100_recv (struct eth_device *dev)
{
	u16 status, stat;
	int rx_prev, length = 0;

	stat = INW (dev, SCBStatus);
	OUTW (dev, stat & SCB_STATUS_RNR, SCBStatus);

	for (;;) {
		status = le16_to_cpu (rx_ring[rx_next].status);

		if (!(status & RFD_STATUS_C)) {
			break;
		}

		/* Valid frame status.
		 */
		if ((status & RFD_STATUS_OK)) {
			/* A valid frame received.
			 */
			length = le32_to_cpu (rx_ring[rx_next].count) & 0x3fff;

			/* Pass the packet up to the protocol
			 * layers.
			 */
			net_process_received_packet((u8 *)rx_ring[rx_next].data,
						    length);
		} else {
			/* There was an error.
			 */
			printf ("RX error status = 0x%08X\n", status);
		}

		rx_ring[rx_next].control = cpu_to_le16 (RFD_CONTROL_S);
		rx_ring[rx_next].status = 0;
		rx_ring[rx_next].count = cpu_to_le32 (PKTSIZE_ALIGN << 16);

		rx_prev = (rx_next + NUM_RX_DESC - 1) % NUM_RX_DESC;
		rx_ring[rx_prev].control = 0;

		/* Update entry information.
		 */
		rx_next = (rx_next + 1) % NUM_RX_DESC;
	}

	if (stat & SCB_STATUS_RNR) {

		printf ("%s: Receiver is not ready, restart it !\n", dev->name);

		/* Reinitialize Rx ring.
		 */
		init_rx_ring (dev);

		if (!wait_for_eepro100 (dev)) {
			printf ("Error: Can not restart ethernet controller.\n");
			goto Done;
		}

		OUTL (dev, phys_to_bus ((u32) & rx_ring[rx_next]), SCBPointer);
		OUTW (dev, SCB_M | RUC_START, SCBCmd);
	}

  Done:
	return length;
}

static void eepro100_halt (struct eth_device *dev)
{
	/* Reset the ethernet controller
	 */
	OUTL (dev, I82559_SELECTIVE_RESET, SCBPort);
	udelay (20);

	OUTL (dev, I82559_RESET, SCBPort);
	udelay (20);

	if (!wait_for_eepro100 (dev)) {
		printf ("Error: Can not reset ethernet controller.\n");
		goto Done;
	}
	OUTL (dev, 0, SCBPointer);
	OUTW (dev, SCB_M | RUC_ADDR_LOAD, SCBCmd);

	if (!wait_for_eepro100 (dev)) {
		printf ("Error: Can not reset ethernet controller.\n");
		goto Done;
	}
	OUTL (dev, 0, SCBPointer);
	OUTW (dev, SCB_M | CU_ADDR_LOAD, SCBCmd);

  Done:
	return;
}

	/* SROM Read.
	 */
static int read_eeprom (struct eth_device *dev, int location, int addr_len)
{
	unsigned short retval = 0;
	int read_cmd = location | EE_READ_CMD;
	int i;

	OUTW (dev, EE_ENB & ~EE_CS, SCBeeprom);
	OUTW (dev, EE_ENB, SCBeeprom);

	/* Shift the read command bits out. */
	for (i = 12; i >= 0; i--) {
		short dataval = (read_cmd & (1 << i)) ? EE_DATA_WRITE : 0;

		OUTW (dev, EE_ENB | dataval, SCBeeprom);
		udelay (1);
		OUTW (dev, EE_ENB | dataval | EE_SHIFT_CLK, SCBeeprom);
		udelay (1);
	}
	OUTW (dev, EE_ENB, SCBeeprom);

	for (i = 15; i >= 0; i--) {
		OUTW (dev, EE_ENB | EE_SHIFT_CLK, SCBeeprom);
		udelay (1);
		retval = (retval << 1) |
				((INW (dev, SCBeeprom) & EE_DATA_READ) ? 1 : 0);
		OUTW (dev, EE_ENB, SCBeeprom);
		udelay (1);
	}

	/* Terminate the EEPROM access. */
	OUTW (dev, EE_ENB & ~EE_CS, SCBeeprom);
	return retval;
}

#ifdef CONFIG_EEPRO100_SROM_WRITE
int eepro100_write_eeprom (struct eth_device* dev, int location, int addr_len, unsigned short data)
{
    unsigned short dataval;
    int enable_cmd = 0x3f | EE_EWENB_CMD;
    int write_cmd  = location | EE_WRITE_CMD;
    int i;
    unsigned long datalong, tmplong;

    OUTW(dev, EE_ENB & ~EE_CS, SCBeeprom);
    udelay(1);
    OUTW(dev, EE_ENB, SCBeeprom);

    /* Shift the enable command bits out. */
    for (i = (addr_len+EE_CMD_BITS-1); i >= 0; i--)
    {
	dataval = (enable_cmd & (1 << i)) ? EE_DATA_WRITE : 0;
	OUTW(dev, EE_ENB | dataval, SCBeeprom);
	udelay(1);
	OUTW(dev, EE_ENB | dataval | EE_SHIFT_CLK, SCBeeprom);
	udelay(1);
    }

    OUTW(dev, EE_ENB, SCBeeprom);
    udelay(1);
    OUTW(dev, EE_ENB & ~EE_CS, SCBeeprom);
    udelay(1);
    OUTW(dev, EE_ENB, SCBeeprom);


    /* Shift the write command bits out. */
    for (i = (addr_len+EE_CMD_BITS-1); i >= 0; i--)
    {
	dataval = (write_cmd & (1 << i)) ? EE_DATA_WRITE : 0;
	OUTW(dev, EE_ENB | dataval, SCBeeprom);
	udelay(1);
	OUTW(dev, EE_ENB | dataval | EE_SHIFT_CLK, SCBeeprom);
	udelay(1);
    }

    /* Write the data */
    datalong= (unsigned long) ((((data) & 0x00ff) << 8) | ( (data) >> 8));

    for (i = 0; i< EE_DATA_BITS; i++)
    {
    /* Extract and move data bit to bit DI */
    dataval = ((datalong & 0x8000)>>13) ? EE_DATA_WRITE : 0;

    OUTW(dev, EE_ENB | dataval, SCBeeprom);
    udelay(1);
    OUTW(dev, EE_ENB | dataval | EE_SHIFT_CLK, SCBeeprom);
    udelay(1);
    OUTW(dev, EE_ENB | dataval, SCBeeprom);
    udelay(1);

    datalong = datalong << 1;	/* Adjust significant data bit*/
    }

    /* Finish up command  (toggle CS) */
    OUTW(dev, EE_ENB & ~EE_CS, SCBeeprom);
    udelay(1);			/* delay for more than 250 ns */
    OUTW(dev, EE_ENB, SCBeeprom);

    /* Wait for programming ready (D0 = 1) */
    tmplong = 10;
    do
    {
	dataval = INW(dev, SCBeeprom);
	if (dataval & EE_DATA_READ)
	    break;
	udelay(10000);
    }
    while (-- tmplong);

    if (tmplong == 0)
    {
	printf ("Write i82559 eeprom timed out (100 ms waiting for data ready.\n");
	return -1;
    }

    /* Terminate the EEPROM access. */
    OUTW(dev, EE_ENB & ~EE_CS, SCBeeprom);

    return 0;
}
#endif

static void init_rx_ring (struct eth_device *dev)
{
	int i;

	for (i = 0; i < NUM_RX_DESC; i++) {
		rx_ring[i].status = 0;
		rx_ring[i].control =
				(i == NUM_RX_DESC - 1) ? cpu_to_le16 (RFD_CONTROL_S) : 0;
		rx_ring[i].link =
				cpu_to_le32 (phys_to_bus
							 ((u32) & rx_ring[(i + 1) % NUM_RX_DESC]));
		rx_ring[i].rx_buf_addr = 0xffffffff;
		rx_ring[i].count = cpu_to_le32 (PKTSIZE_ALIGN << 16);
	}

	rx_next = 0;
}

static void purge_tx_ring (struct eth_device *dev)
{
	int i;

	tx_next = 0;
	tx_threshold = 0x01208000;

	for (i = 0; i < NUM_TX_DESC; i++) {
		tx_ring[i].status = 0;
		tx_ring[i].command = 0;
		tx_ring[i].link = 0;
		tx_ring[i].tx_desc_addr = 0;
		tx_ring[i].count = 0;

		tx_ring[i].tx_buf_addr0 = 0;
		tx_ring[i].tx_buf_size0 = 0;
		tx_ring[i].tx_buf_addr1 = 0;
		tx_ring[i].tx_buf_size1 = 0;
	}
}

static void read_hw_addr (struct eth_device *dev, bd_t * bis)
{
	u16 sum = 0;
	int i, j;
	int addr_len = read_eeprom (dev, 0, 6) == 0xffff ? 8 : 6;

	for (j = 0, i = 0; i < 0x40; i++) {
		u16 value = read_eeprom (dev, i, addr_len);

		sum += value;
		if (i < 3) {
			dev->enetaddr[j++] = value;
			dev->enetaddr[j++] = value >> 8;
		}
	}

	if (sum != 0xBABA) {
		memset (dev->enetaddr, 0, ETH_ALEN);
#ifdef DEBUG
		printf ("%s: Invalid EEPROM checksum %#4.4x, "
			"check settings before activating this device!\n",
			dev->name, sum);
#endif
	}
}
