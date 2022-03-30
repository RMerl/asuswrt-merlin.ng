// SPDX-License-Identifier: GPL-2.0+

#include <common.h>
#include <malloc.h>
#include <net.h>
#include <netdev.h>
#include <pci.h>

#undef DEBUG_SROM
#undef DEBUG_SROM2

#undef UPDATE_SROM

/* PCI Registers.
 */
#define PCI_CFDA_PSM		0x43

#define CFRV_RN		0x000000f0	/* Revision Number */

#define WAKEUP		0x00		/* Power Saving Wakeup */
#define SLEEP		0x80		/* Power Saving Sleep Mode */

#define DC2114x_BRK	0x0020		/* CFRV break between DC21142 & DC21143 */

/* Ethernet chip registers.
 */
#define DE4X5_BMR	0x000		/* Bus Mode Register */
#define DE4X5_TPD	0x008		/* Transmit Poll Demand Reg */
#define DE4X5_RRBA	0x018		/* RX Ring Base Address Reg */
#define DE4X5_TRBA	0x020		/* TX Ring Base Address Reg */
#define DE4X5_STS	0x028		/* Status Register */
#define DE4X5_OMR	0x030		/* Operation Mode Register */
#define DE4X5_SICR	0x068		/* SIA Connectivity Register */
#define DE4X5_APROM	0x048		/* Ethernet Address PROM */

/* Register bits.
 */
#define BMR_SWR		0x00000001	/* Software Reset */
#define STS_TS		0x00700000	/* Transmit Process State */
#define STS_RS		0x000e0000	/* Receive Process State */
#define OMR_ST		0x00002000	/* Start/Stop Transmission Command */
#define OMR_SR		0x00000002	/* Start/Stop Receive */
#define OMR_PS		0x00040000	/* Port Select */
#define OMR_SDP		0x02000000	/* SD Polarity - MUST BE ASSERTED */
#define OMR_PM		0x00000080	/* Pass All Multicast */

/* Descriptor bits.
 */
#define R_OWN		0x80000000	/* Own Bit */
#define RD_RER		0x02000000	/* Receive End Of Ring */
#define RD_LS		0x00000100	/* Last Descriptor */
#define RD_ES		0x00008000	/* Error Summary */
#define TD_TER		0x02000000	/* Transmit End Of Ring */
#define T_OWN		0x80000000	/* Own Bit */
#define TD_LS		0x40000000	/* Last Segment */
#define TD_FS		0x20000000	/* First Segment */
#define TD_ES		0x00008000	/* Error Summary */
#define TD_SET		0x08000000	/* Setup Packet */

/* The EEPROM commands include the alway-set leading bit. */
#define SROM_WRITE_CMD	5
#define SROM_READ_CMD	6
#define SROM_ERASE_CMD	7

#define SROM_HWADD	    0x0014	/* Hardware Address offset in SROM */
#define SROM_RD		0x00004000	/* Read from Boot ROM */
#define EE_DATA_WRITE	      0x04	/* EEPROM chip data in. */
#define EE_WRITE_0	    0x4801
#define EE_WRITE_1	    0x4805
#define EE_DATA_READ	      0x08	/* EEPROM chip data out. */
#define SROM_SR		0x00000800	/* Select Serial ROM when set */

#define DT_IN		0x00000004	/* Serial Data In */
#define DT_CLK		0x00000002	/* Serial ROM Clock */
#define DT_CS		0x00000001	/* Serial ROM Chip Select */

#define POLL_DEMAND	1

#ifdef CONFIG_TULIP_FIX_DAVICOM
#define RESET_DM9102(dev) {\
    unsigned long i;\
    i=INL(dev, 0x0);\
    udelay(1000);\
    OUTL(dev, i | BMR_SWR, DE4X5_BMR);\
    udelay(1000);\
}
#else
#define RESET_DE4X5(dev) {\
    int i;\
    i=INL(dev, DE4X5_BMR);\
    udelay(1000);\
    OUTL(dev, i | BMR_SWR, DE4X5_BMR);\
    udelay(1000);\
    OUTL(dev, i, DE4X5_BMR);\
    udelay(1000);\
    for (i=0;i<5;i++) {INL(dev, DE4X5_BMR); udelay(10000);}\
    udelay(1000);\
}
#endif

#define START_DE4X5(dev) {\
    s32 omr; \
    omr = INL(dev, DE4X5_OMR);\
    omr |= OMR_ST | OMR_SR;\
    OUTL(dev, omr, DE4X5_OMR);		/* Enable the TX and/or RX */\
}

#define STOP_DE4X5(dev) {\
    s32 omr; \
    omr = INL(dev, DE4X5_OMR);\
    omr &= ~(OMR_ST|OMR_SR);\
    OUTL(dev, omr, DE4X5_OMR);		/* Disable the TX and/or RX */ \
}

#define NUM_RX_DESC PKTBUFSRX
#ifndef CONFIG_TULIP_FIX_DAVICOM
	#define NUM_TX_DESC 1			/* Number of TX descriptors   */
#else
	#define NUM_TX_DESC 4
#endif
#define RX_BUFF_SZ  PKTSIZE_ALIGN

#define TOUT_LOOP   1000000

#define SETUP_FRAME_LEN 192

struct de4x5_desc {
	volatile s32 status;
	u32 des1;
	u32 buf;
	u32 next;
};

static struct de4x5_desc rx_ring[NUM_RX_DESC] __attribute__ ((aligned(32))); /* RX descriptor ring         */
static struct de4x5_desc tx_ring[NUM_TX_DESC] __attribute__ ((aligned(32))); /* TX descriptor ring         */
static int rx_new;                             /* RX descriptor ring pointer */
static int tx_new;                             /* TX descriptor ring pointer */

static char rxRingSize;
static char txRingSize;

#if defined(UPDATE_SROM) || !defined(CONFIG_TULIP_FIX_DAVICOM)
static void  sendto_srom(struct eth_device* dev, u_int command, u_long addr);
static int   getfrom_srom(struct eth_device* dev, u_long addr);
static int   do_eeprom_cmd(struct eth_device *dev, u_long ioaddr,int cmd,int cmd_len);
static int   do_read_eeprom(struct eth_device *dev,u_long ioaddr,int location,int addr_len);
#endif	/* UPDATE_SROM || !CONFIG_TULIP_FIX_DAVICOM */
#ifdef UPDATE_SROM
static int   write_srom(struct eth_device *dev, u_long ioaddr, int index, int new_value);
static void  update_srom(struct eth_device *dev, bd_t *bis);
#endif
#ifndef CONFIG_TULIP_FIX_DAVICOM
static int   read_srom(struct eth_device *dev, u_long ioaddr, int index);
static void  read_hw_addr(struct eth_device* dev, bd_t * bis);
#endif	/* CONFIG_TULIP_FIX_DAVICOM */
static void  send_setup_frame(struct eth_device* dev, bd_t * bis);

static int   dc21x4x_init(struct eth_device* dev, bd_t* bis);
static int   dc21x4x_send(struct eth_device *dev, void *packet, int length);
static int   dc21x4x_recv(struct eth_device* dev);
static void  dc21x4x_halt(struct eth_device* dev);
#ifdef CONFIG_TULIP_SELECT_MEDIA
extern void  dc21x4x_select_media(struct eth_device* dev);
#endif

#if defined(CONFIG_E500)
#define phys_to_bus(a) (a)
#else
#define phys_to_bus(a)	pci_phys_to_mem((pci_dev_t)dev->priv, a)
#endif

static int INL(struct eth_device* dev, u_long addr)
{
	return le32_to_cpu(*(volatile u_long *)(addr + dev->iobase));
}

static void OUTL(struct eth_device* dev, int command, u_long addr)
{
	*(volatile u_long *)(addr + dev->iobase) = cpu_to_le32(command);
}

static struct pci_device_id supported[] = {
	{ PCI_VENDOR_ID_DEC, PCI_DEVICE_ID_DEC_TULIP_FAST },
	{ PCI_VENDOR_ID_DEC, PCI_DEVICE_ID_DEC_21142 },
#ifdef CONFIG_TULIP_FIX_DAVICOM
	{ PCI_VENDOR_ID_DAVICOM, PCI_DEVICE_ID_DAVICOM_DM9102A },
#endif
	{ }
};

int dc21x4x_initialize(bd_t *bis)
{
	int			idx=0;
	int			card_number = 0;
	unsigned int		cfrv;
	unsigned char		timer;
	pci_dev_t		devbusfn;
	unsigned int		iobase;
	unsigned short		status;
	struct eth_device*	dev;

	while(1) {
		devbusfn =  pci_find_devices(supported, idx++);
		if (devbusfn == -1) {
			break;
		}

		/* Get the chip configuration revision register. */
		pci_read_config_dword(devbusfn, PCI_REVISION_ID, &cfrv);

#ifndef CONFIG_TULIP_FIX_DAVICOM
		if ((cfrv & CFRV_RN) < DC2114x_BRK ) {
			printf("Error: The chip is not DC21143.\n");
			continue;
		}
#endif

		pci_read_config_word(devbusfn, PCI_COMMAND, &status);
		status |=
#ifdef CONFIG_TULIP_USE_IO
		  PCI_COMMAND_IO |
#else
		  PCI_COMMAND_MEMORY |
#endif
		  PCI_COMMAND_MASTER;
		pci_write_config_word(devbusfn, PCI_COMMAND, status);

		pci_read_config_word(devbusfn, PCI_COMMAND, &status);
#ifdef CONFIG_TULIP_USE_IO
		if (!(status & PCI_COMMAND_IO)) {
			printf("Error: Can not enable I/O access.\n");
			continue;
		}
#else
		if (!(status & PCI_COMMAND_MEMORY)) {
			printf("Error: Can not enable MEMORY access.\n");
			continue;
		}
#endif

		if (!(status & PCI_COMMAND_MASTER)) {
			printf("Error: Can not enable Bus Mastering.\n");
			continue;
		}

		/* Check the latency timer for values >= 0x60. */
		pci_read_config_byte(devbusfn, PCI_LATENCY_TIMER, &timer);

		if (timer < 0x60) {
			pci_write_config_byte(devbusfn, PCI_LATENCY_TIMER, 0x60);
		}

#ifdef CONFIG_TULIP_USE_IO
		/* read BAR for memory space access */
		pci_read_config_dword(devbusfn, PCI_BASE_ADDRESS_0, &iobase);
		iobase &= PCI_BASE_ADDRESS_IO_MASK;
#else
		/* read BAR for memory space access */
		pci_read_config_dword(devbusfn, PCI_BASE_ADDRESS_1, &iobase);
		iobase &= PCI_BASE_ADDRESS_MEM_MASK;
#endif
		debug ("dc21x4x: DEC 21142 PCI Device @0x%x\n", iobase);

		dev = (struct eth_device*) malloc(sizeof *dev);

		if (!dev) {
			printf("Can not allocalte memory of dc21x4x\n");
			break;
		}
		memset(dev, 0, sizeof(*dev));

#ifdef CONFIG_TULIP_FIX_DAVICOM
		sprintf(dev->name, "Davicom#%d", card_number);
#else
		sprintf(dev->name, "dc21x4x#%d", card_number);
#endif

#ifdef CONFIG_TULIP_USE_IO
		dev->iobase = pci_io_to_phys(devbusfn, iobase);
#else
		dev->iobase = pci_mem_to_phys(devbusfn, iobase);
#endif
		dev->priv   = (void*) devbusfn;
		dev->init   = dc21x4x_init;
		dev->halt   = dc21x4x_halt;
		dev->send   = dc21x4x_send;
		dev->recv   = dc21x4x_recv;

		/* Ensure we're not sleeping. */
		pci_write_config_byte(devbusfn, PCI_CFDA_PSM, WAKEUP);

		udelay(10 * 1000);

#ifndef CONFIG_TULIP_FIX_DAVICOM
		read_hw_addr(dev, bis);
#endif
		eth_register(dev);

		card_number++;
	}

	return card_number;
}

static int dc21x4x_init(struct eth_device* dev, bd_t* bis)
{
	int		i;
	int		devbusfn = (int) dev->priv;

	/* Ensure we're not sleeping. */
	pci_write_config_byte(devbusfn, PCI_CFDA_PSM, WAKEUP);

#ifdef CONFIG_TULIP_FIX_DAVICOM
	RESET_DM9102(dev);
#else
	RESET_DE4X5(dev);
#endif

	if ((INL(dev, DE4X5_STS) & (STS_TS | STS_RS)) != 0) {
		printf("Error: Cannot reset ethernet controller.\n");
		return -1;
	}

#ifdef CONFIG_TULIP_SELECT_MEDIA
	dc21x4x_select_media(dev);
#else
	OUTL(dev, OMR_SDP | OMR_PS | OMR_PM, DE4X5_OMR);
#endif

	for (i = 0; i < NUM_RX_DESC; i++) {
		rx_ring[i].status = cpu_to_le32(R_OWN);
		rx_ring[i].des1 = cpu_to_le32(RX_BUFF_SZ);
		rx_ring[i].buf = cpu_to_le32(
			phys_to_bus((u32)net_rx_packets[i]));
#ifdef CONFIG_TULIP_FIX_DAVICOM
		rx_ring[i].next = cpu_to_le32(
			phys_to_bus((u32)&rx_ring[(i + 1) % NUM_RX_DESC]));
#else
		rx_ring[i].next = 0;
#endif
	}

	for (i=0; i < NUM_TX_DESC; i++) {
		tx_ring[i].status = 0;
		tx_ring[i].des1 = 0;
		tx_ring[i].buf = 0;

#ifdef CONFIG_TULIP_FIX_DAVICOM
	tx_ring[i].next = cpu_to_le32(phys_to_bus((u32) &tx_ring[(i+1) % NUM_TX_DESC]));
#else
		tx_ring[i].next = 0;
#endif
	}

	rxRingSize = NUM_RX_DESC;
	txRingSize = NUM_TX_DESC;

	/* Write the end of list marker to the descriptor lists. */
	rx_ring[rxRingSize - 1].des1 |= cpu_to_le32(RD_RER);
	tx_ring[txRingSize - 1].des1 |= cpu_to_le32(TD_TER);

	/* Tell the adapter where the TX/RX rings are located. */
	OUTL(dev, phys_to_bus((u32) &rx_ring), DE4X5_RRBA);
	OUTL(dev, phys_to_bus((u32) &tx_ring), DE4X5_TRBA);

	START_DE4X5(dev);

	tx_new = 0;
	rx_new = 0;

	send_setup_frame(dev, bis);

	return 0;
}

static int dc21x4x_send(struct eth_device *dev, void *packet, int length)
{
	int		status = -1;
	int		i;

	if (length <= 0) {
		printf("%s: bad packet size: %d\n", dev->name, length);
		goto Done;
	}

	for(i = 0; tx_ring[tx_new].status & cpu_to_le32(T_OWN); i++) {
		if (i >= TOUT_LOOP) {
			printf("%s: tx error buffer not ready\n", dev->name);
			goto Done;
		}
	}

	tx_ring[tx_new].buf    = cpu_to_le32(phys_to_bus((u32) packet));
	tx_ring[tx_new].des1   = cpu_to_le32(TD_TER | TD_LS | TD_FS | length);
	tx_ring[tx_new].status = cpu_to_le32(T_OWN);

	OUTL(dev, POLL_DEMAND, DE4X5_TPD);

	for(i = 0; tx_ring[tx_new].status & cpu_to_le32(T_OWN); i++) {
		if (i >= TOUT_LOOP) {
			printf(".%s: tx buffer not ready\n", dev->name);
			goto Done;
		}
	}

	if (le32_to_cpu(tx_ring[tx_new].status) & TD_ES) {
#if 0 /* test-only */
		printf("TX error status = 0x%08X\n",
			le32_to_cpu(tx_ring[tx_new].status));
#endif
		tx_ring[tx_new].status = 0x0;
		goto Done;
	}

	status = length;

 Done:
    tx_new = (tx_new+1) % NUM_TX_DESC;
	return status;
}

static int dc21x4x_recv(struct eth_device* dev)
{
	s32		status;
	int		length    = 0;

	for ( ; ; ) {
		status = (s32)le32_to_cpu(rx_ring[rx_new].status);

		if (status & R_OWN) {
			break;
		}

		if (status & RD_LS) {
			/* Valid frame status.
			 */
			if (status & RD_ES) {

				/* There was an error.
				 */
				printf("RX error status = 0x%08X\n", status);
			} else {
				/* A valid frame received.
				 */
				length = (le32_to_cpu(rx_ring[rx_new].status) >> 16);

				/* Pass the packet up to the protocol
				 * layers.
				 */
				net_process_received_packet(
					net_rx_packets[rx_new], length - 4);
			}

			/* Change buffer ownership for this frame, back
			 * to the adapter.
			 */
			rx_ring[rx_new].status = cpu_to_le32(R_OWN);
		}

		/* Update entry information.
		 */
		rx_new = (rx_new + 1) % rxRingSize;
	}

	return length;
}

static void dc21x4x_halt(struct eth_device* dev)
{
	int		devbusfn = (int) dev->priv;

	STOP_DE4X5(dev);
	OUTL(dev, 0, DE4X5_SICR);

	pci_write_config_byte(devbusfn, PCI_CFDA_PSM, SLEEP);
}

static void send_setup_frame(struct eth_device* dev, bd_t *bis)
{
	int		i;
	char	setup_frame[SETUP_FRAME_LEN];
	char	*pa = &setup_frame[0];

	memset(pa, 0xff, SETUP_FRAME_LEN);

	for (i = 0; i < ETH_ALEN; i++) {
		*(pa + (i & 1)) = dev->enetaddr[i];
		if (i & 0x01) {
			pa += 4;
		}
	}

	for(i = 0; tx_ring[tx_new].status & cpu_to_le32(T_OWN); i++) {
		if (i >= TOUT_LOOP) {
			printf("%s: tx error buffer not ready\n", dev->name);
			goto Done;
		}
	}

	tx_ring[tx_new].buf = cpu_to_le32(phys_to_bus((u32) &setup_frame[0]));
	tx_ring[tx_new].des1 = cpu_to_le32(TD_TER | TD_SET| SETUP_FRAME_LEN);
	tx_ring[tx_new].status = cpu_to_le32(T_OWN);

	OUTL(dev, POLL_DEMAND, DE4X5_TPD);

	for(i = 0; tx_ring[tx_new].status & cpu_to_le32(T_OWN); i++) {
		if (i >= TOUT_LOOP) {
			printf("%s: tx buffer not ready\n", dev->name);
			goto Done;
		}
	}

	if (le32_to_cpu(tx_ring[tx_new].status) != 0x7FFFFFFF) {
		printf("TX error status2 = 0x%08X\n", le32_to_cpu(tx_ring[tx_new].status));
	}
	tx_new = (tx_new+1) % NUM_TX_DESC;

Done:
	return;
}

#if defined(UPDATE_SROM) || !defined(CONFIG_TULIP_FIX_DAVICOM)
/* SROM Read and write routines.
 */
static void
sendto_srom(struct eth_device* dev, u_int command, u_long addr)
{
	OUTL(dev, command, addr);
	udelay(1);
}

static int
getfrom_srom(struct eth_device* dev, u_long addr)
{
	s32 tmp;

	tmp = INL(dev, addr);
	udelay(1);

	return tmp;
}

/* Note: this routine returns extra data bits for size detection. */
static int do_read_eeprom(struct eth_device *dev, u_long ioaddr, int location, int addr_len)
{
	int i;
	unsigned retval = 0;
	int read_cmd = location | (SROM_READ_CMD << addr_len);

	sendto_srom(dev, SROM_RD | SROM_SR, ioaddr);
	sendto_srom(dev, SROM_RD | SROM_SR | DT_CS, ioaddr);

#ifdef DEBUG_SROM
	printf(" EEPROM read at %d ", location);
#endif

	/* Shift the read command bits out. */
	for (i = 4 + addr_len; i >= 0; i--) {
		short dataval = (read_cmd & (1 << i)) ? EE_DATA_WRITE : 0;
		sendto_srom(dev, SROM_RD | SROM_SR | DT_CS | dataval, ioaddr);
		udelay(10);
		sendto_srom(dev, SROM_RD | SROM_SR | DT_CS | dataval | DT_CLK, ioaddr);
		udelay(10);
#ifdef DEBUG_SROM2
		printf("%X", getfrom_srom(dev, ioaddr) & 15);
#endif
		retval = (retval << 1) | ((getfrom_srom(dev, ioaddr) & EE_DATA_READ) ? 1 : 0);
	}

	sendto_srom(dev, SROM_RD | SROM_SR | DT_CS, ioaddr);

#ifdef DEBUG_SROM2
	printf(" :%X:", getfrom_srom(dev, ioaddr) & 15);
#endif

	for (i = 16; i > 0; i--) {
		sendto_srom(dev, SROM_RD | SROM_SR | DT_CS | DT_CLK, ioaddr);
		udelay(10);
#ifdef DEBUG_SROM2
		printf("%X", getfrom_srom(dev, ioaddr) & 15);
#endif
		retval = (retval << 1) | ((getfrom_srom(dev, ioaddr) & EE_DATA_READ) ? 1 : 0);
		sendto_srom(dev, SROM_RD | SROM_SR | DT_CS, ioaddr);
		udelay(10);
	}

	/* Terminate the EEPROM access. */
	sendto_srom(dev, SROM_RD | SROM_SR, ioaddr);

#ifdef DEBUG_SROM2
	printf(" EEPROM value at %d is %5.5x.\n", location, retval);
#endif

	return retval;
}
#endif	/* UPDATE_SROM || !CONFIG_TULIP_FIX_DAVICOM */

/* This executes a generic EEPROM command, typically a write or write
 * enable. It returns the data output from the EEPROM, and thus may
 * also be used for reads.
 */
#if defined(UPDATE_SROM) || !defined(CONFIG_TULIP_FIX_DAVICOM)
static int do_eeprom_cmd(struct eth_device *dev, u_long ioaddr, int cmd, int cmd_len)
{
	unsigned retval = 0;

#ifdef DEBUG_SROM
	printf(" EEPROM op 0x%x: ", cmd);
#endif

	sendto_srom(dev,SROM_RD | SROM_SR | DT_CS | DT_CLK, ioaddr);

	/* Shift the command bits out. */
	do {
		short dataval = (cmd & (1 << cmd_len)) ? EE_WRITE_1 : EE_WRITE_0;
		sendto_srom(dev,dataval, ioaddr);
		udelay(10);

#ifdef DEBUG_SROM2
		printf("%X", getfrom_srom(dev,ioaddr) & 15);
#endif

		sendto_srom(dev,dataval | DT_CLK, ioaddr);
		udelay(10);
		retval = (retval << 1) | ((getfrom_srom(dev,ioaddr) & EE_DATA_READ) ? 1 : 0);
	} while (--cmd_len >= 0);
	sendto_srom(dev,SROM_RD | SROM_SR | DT_CS, ioaddr);

	/* Terminate the EEPROM access. */
	sendto_srom(dev,SROM_RD | SROM_SR, ioaddr);

#ifdef DEBUG_SROM
	printf(" EEPROM result is 0x%5.5x.\n", retval);
#endif

	return retval;
}
#endif	/* UPDATE_SROM || !CONFIG_TULIP_FIX_DAVICOM */

#ifndef CONFIG_TULIP_FIX_DAVICOM
static int read_srom(struct eth_device *dev, u_long ioaddr, int index)
{
	int ee_addr_size = do_read_eeprom(dev, ioaddr, 0xff, 8) & 0x40000 ? 8 : 6;

	return do_eeprom_cmd(dev, ioaddr,
			     (((SROM_READ_CMD << ee_addr_size) | index) << 16)
			     | 0xffff, 3 + ee_addr_size + 16);
}
#endif	/* CONFIG_TULIP_FIX_DAVICOM */

#ifdef UPDATE_SROM
static int write_srom(struct eth_device *dev, u_long ioaddr, int index, int new_value)
{
	int ee_addr_size = do_read_eeprom(dev, ioaddr, 0xff, 8) & 0x40000 ? 8 : 6;
	int i;
	unsigned short newval;

	udelay(10*1000); /* test-only */

#ifdef DEBUG_SROM
	printf("ee_addr_size=%d.\n", ee_addr_size);
	printf("Writing new entry 0x%4.4x to offset %d.\n", new_value, index);
#endif

	/* Enable programming modes. */
	do_eeprom_cmd(dev, ioaddr, (0x4f << (ee_addr_size-4)), 3+ee_addr_size);

	/* Do the actual write. */
	do_eeprom_cmd(dev, ioaddr,
		      (((SROM_WRITE_CMD<<ee_addr_size)|index) << 16) | new_value,
		      3 + ee_addr_size + 16);

	/* Poll for write finished. */
	sendto_srom(dev, SROM_RD | SROM_SR | DT_CS, ioaddr);
	for (i = 0; i < 10000; i++)			/* Typical 2000 ticks */
		if (getfrom_srom(dev, ioaddr) & EE_DATA_READ)
			break;

#ifdef DEBUG_SROM
	printf(" Write finished after %d ticks.\n", i);
#endif

	/* Disable programming. */
	do_eeprom_cmd(dev, ioaddr, (0x40 << (ee_addr_size-4)), 3 + ee_addr_size);

	/* And read the result. */
	newval = do_eeprom_cmd(dev, ioaddr,
			       (((SROM_READ_CMD<<ee_addr_size)|index) << 16)
			       | 0xffff, 3 + ee_addr_size + 16);
#ifdef DEBUG_SROM
	printf("  New value at offset %d is %4.4x.\n", index, newval);
#endif
	return 1;
}
#endif

#ifndef CONFIG_TULIP_FIX_DAVICOM
static void read_hw_addr(struct eth_device *dev, bd_t *bis)
{
	u_short tmp, *p = (u_short *)(&dev->enetaddr[0]);
	int i, j = 0;

	for (i = 0; i < (ETH_ALEN >> 1); i++) {
		tmp = read_srom(dev, DE4X5_APROM, ((SROM_HWADD >> 1) + i));
		*p = le16_to_cpu(tmp);
		j += *p++;
	}

	if ((j == 0) || (j == 0x2fffd)) {
		memset (dev->enetaddr, 0, ETH_ALEN);
		debug ("Warning: can't read HW address from SROM.\n");
		goto Done;
	}

	return;

Done:
#ifdef UPDATE_SROM
	update_srom(dev, bis);
#endif
	return;
}
#endif	/* CONFIG_TULIP_FIX_DAVICOM */

#ifdef UPDATE_SROM
static void update_srom(struct eth_device *dev, bd_t *bis)
{
	int i;
	static unsigned short eeprom[0x40] = {
		0x140b, 0x6610, 0x0000, 0x0000,	/* 00 */
		0x0000, 0x0000, 0x0000, 0x0000,	/* 04 */
		0x00a3, 0x0103, 0x0000, 0x0000,	/* 08 */
		0x0000, 0x1f00, 0x0000, 0x0000,	/* 0c */
		0x0108, 0x038d, 0x0000, 0x0000,	/* 10 */
		0xe078, 0x0001, 0x0040, 0x0018,	/* 14 */
		0x0000, 0x0000, 0x0000, 0x0000,	/* 18 */
		0x0000, 0x0000, 0x0000, 0x0000,	/* 1c */
		0x0000, 0x0000, 0x0000, 0x0000,	/* 20 */
		0x0000, 0x0000, 0x0000, 0x0000,	/* 24 */
		0x0000, 0x0000, 0x0000, 0x0000,	/* 28 */
		0x0000, 0x0000, 0x0000, 0x0000,	/* 2c */
		0x0000, 0x0000, 0x0000, 0x0000,	/* 30 */
		0x0000, 0x0000, 0x0000, 0x0000,	/* 34 */
		0x0000, 0x0000, 0x0000, 0x0000,	/* 38 */
		0x0000, 0x0000, 0x0000, 0x4e07,	/* 3c */
	};
	uchar enetaddr[6];

	/* Ethernet Addr... */
	if (!eth_env_get_enetaddr("ethaddr", enetaddr))
		return;
	eeprom[0x0a] = (enetaddr[1] << 8) | enetaddr[0];
	eeprom[0x0b] = (enetaddr[3] << 8) | enetaddr[2];
	eeprom[0x0c] = (enetaddr[5] << 8) | enetaddr[4];

	for (i=0; i<0x40; i++) {
		write_srom(dev, DE4X5_APROM, i, eeprom[i]);
	}
}
#endif	/* UPDATE_SROM */
