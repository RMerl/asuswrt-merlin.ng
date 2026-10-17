// SPDX-License-Identifier: GPL-2.0+

#include <common.h>
#include <asm/io.h>
#include <dm.h>
#include <malloc.h>
#include <net.h>
#include <netdev.h>
#include <pci.h>
#include <linux/bitops.h>
#include <linux/delay.h>

#define SROM_DLEVEL	0

/* PCI Registers. */
#define PCI_CFDA_PSM	0x43

#define CFRV_RN		0x000000f0	/* Revision Number */

#define WAKEUP		0x00		/* Power Saving Wakeup */
#define SLEEP		0x80		/* Power Saving Sleep Mode */

#define DC2114x_BRK	0x0020	/* CFRV break between DC21142 & DC21143 */

/* Ethernet chip registers. */
#define DE4X5_BMR	0x000		/* Bus Mode Register */
#define DE4X5_TPD	0x008		/* Transmit Poll Demand Reg */
#define DE4X5_RRBA	0x018		/* RX Ring Base Address Reg */
#define DE4X5_TRBA	0x020		/* TX Ring Base Address Reg */
#define DE4X5_STS	0x028		/* Status Register */
#define DE4X5_OMR	0x030		/* Operation Mode Register */
#define DE4X5_SICR	0x068		/* SIA Connectivity Register */
#define DE4X5_APROM	0x048		/* Ethernet Address PROM */

/* Register bits. */
#define BMR_SWR		0x00000001	/* Software Reset */
#define STS_TS		0x00700000	/* Transmit Process State */
#define STS_RS		0x000e0000	/* Receive Process State */
#define OMR_ST		0x00002000	/* Start/Stop Transmission Command */
#define OMR_SR		0x00000002	/* Start/Stop Receive */
#define OMR_PS		0x00040000	/* Port Select */
#define OMR_SDP		0x02000000	/* SD Polarity - MUST BE ASSERTED */
#define OMR_PM		0x00000080	/* Pass All Multicast */

/* Descriptor bits. */
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

#define SROM_HWADD	0x0014		/* Hardware Address offset in SROM */
#define SROM_RD		0x00004000	/* Read from Boot ROM */
#define EE_DATA_WRITE	0x04		/* EEPROM chip data in. */
#define EE_WRITE_0	0x4801
#define EE_WRITE_1	0x4805
#define EE_DATA_READ	0x08		/* EEPROM chip data out. */
#define SROM_SR		0x00000800	/* Select Serial ROM when set */

#define DT_IN		0x00000004	/* Serial Data In */
#define DT_CLK		0x00000002	/* Serial ROM Clock */
#define DT_CS		0x00000001	/* Serial ROM Chip Select */

#define POLL_DEMAND	1

#define phys_to_bus(dev, a)	dm_pci_phys_to_mem((dev), (a))

#define NUM_RX_DESC PKTBUFSRX
#define NUM_TX_DESC 1			/* Number of TX descriptors   */
#define RX_BUFF_SZ  PKTSIZE_ALIGN

#define TOUT_LOOP   1000000

#define SETUP_FRAME_LEN 192

struct de4x5_desc {
	volatile s32 status;
	u32 des1;
	u32 buf;
	u32 next;
};

struct dc2114x_priv {
	struct de4x5_desc rx_ring[NUM_RX_DESC] __aligned(32);
	struct de4x5_desc tx_ring[NUM_TX_DESC] __aligned(32);
	int rx_new;	/* RX descriptor ring pointer */
	int tx_new;	/* TX descriptor ring pointer */
	char rx_ring_size;
	char tx_ring_size;
	struct udevice		*devno;
	char			*name;
	void __iomem		*iobase;
	u8			*enetaddr;
};

/* RX and TX descriptor ring */
static u32 dc2114x_inl(struct dc2114x_priv *priv, u32 addr)
{
	return le32_to_cpu(readl(priv->iobase + addr));
}

static void dc2114x_outl(struct dc2114x_priv *priv, u32 command, u32 addr)
{
	writel(cpu_to_le32(command), priv->iobase + addr);
}

static void reset_de4x5(struct dc2114x_priv *priv)
{
	u32 i;

	i = dc2114x_inl(priv, DE4X5_BMR);
	mdelay(1);
	dc2114x_outl(priv, i | BMR_SWR, DE4X5_BMR);
	mdelay(1);
	dc2114x_outl(priv, i, DE4X5_BMR);
	mdelay(1);

	for (i = 0; i < 5; i++) {
		dc2114x_inl(priv, DE4X5_BMR);
		mdelay(10);
	}

	mdelay(1);
}

static void start_de4x5(struct dc2114x_priv *priv)
{
	u32 omr;

	omr = dc2114x_inl(priv, DE4X5_OMR);
	omr |= OMR_ST | OMR_SR;
	dc2114x_outl(priv, omr, DE4X5_OMR);	/* Enable the TX and/or RX */
}

static void stop_de4x5(struct dc2114x_priv *priv)
{
	u32 omr;

	omr = dc2114x_inl(priv, DE4X5_OMR);
	omr &= ~(OMR_ST | OMR_SR);
	dc2114x_outl(priv, omr, DE4X5_OMR);	/* Disable the TX and/or RX */
}

/* SROM Read and write routines. */
static void sendto_srom(struct dc2114x_priv *priv, u_int command, u_long addr)
{
	dc2114x_outl(priv, command, addr);
	udelay(1);
}

static int getfrom_srom(struct dc2114x_priv *priv, u_long addr)
{
	u32 tmp = dc2114x_inl(priv, addr);

	udelay(1);
	return tmp;
}

/* Note: this routine returns extra data bits for size detection. */
static int do_read_eeprom(struct dc2114x_priv *priv, u_long ioaddr, int location,
			  int addr_len)
{
	int read_cmd = location | (SROM_READ_CMD << addr_len);
	unsigned int retval = 0;
	int i;

	sendto_srom(priv, SROM_RD | SROM_SR, ioaddr);
	sendto_srom(priv, SROM_RD | SROM_SR | DT_CS, ioaddr);

	debug_cond(SROM_DLEVEL >= 1, " EEPROM read at %d ", location);

	/* Shift the read command bits out. */
	for (i = 4 + addr_len; i >= 0; i--) {
		short dataval = (read_cmd & (1 << i)) ? EE_DATA_WRITE : 0;

		sendto_srom(priv, SROM_RD | SROM_SR | DT_CS | dataval,
			    ioaddr);
		udelay(10);
		sendto_srom(priv, SROM_RD | SROM_SR | DT_CS | dataval | DT_CLK,
			    ioaddr);
		udelay(10);
		debug_cond(SROM_DLEVEL >= 2, "%X",
			   getfrom_srom(priv, ioaddr) & 15);
		retval = (retval << 1) |
			 !!(getfrom_srom(priv, ioaddr) & EE_DATA_READ);
	}

	sendto_srom(priv, SROM_RD | SROM_SR | DT_CS, ioaddr);

	debug_cond(SROM_DLEVEL >= 2, " :%X:", getfrom_srom(priv, ioaddr) & 15);

	for (i = 16; i > 0; i--) {
		sendto_srom(priv, SROM_RD | SROM_SR | DT_CS | DT_CLK, ioaddr);
		udelay(10);
		debug_cond(SROM_DLEVEL >= 2, "%X",
			   getfrom_srom(priv, ioaddr) & 15);
		retval = (retval << 1) |
			 !!(getfrom_srom(priv, ioaddr) & EE_DATA_READ);
		sendto_srom(priv, SROM_RD | SROM_SR | DT_CS, ioaddr);
		udelay(10);
	}

	/* Terminate the EEPROM access. */
	sendto_srom(priv, SROM_RD | SROM_SR, ioaddr);

	debug_cond(SROM_DLEVEL >= 2, " EEPROM value at %d is %5.5x.\n",
		   location, retval);

	return retval;
}

/*
 * This executes a generic EEPROM command, typically a write or write
 * enable. It returns the data output from the EEPROM, and thus may
 * also be used for reads.
 */
static int do_eeprom_cmd(struct dc2114x_priv *priv, u_long ioaddr, int cmd,
			 int cmd_len)
{
	unsigned int retval = 0;

	debug_cond(SROM_DLEVEL >= 1, " EEPROM op 0x%x: ", cmd);

	sendto_srom(priv, SROM_RD | SROM_SR | DT_CS | DT_CLK, ioaddr);

	/* Shift the command bits out. */
	do {
		short dataval = (cmd & BIT(cmd_len)) ? EE_WRITE_1 : EE_WRITE_0;

		sendto_srom(priv, dataval, ioaddr);
		udelay(10);

		debug_cond(SROM_DLEVEL >= 2, "%X",
			   getfrom_srom(priv, ioaddr) & 15);

		sendto_srom(priv, dataval | DT_CLK, ioaddr);
		udelay(10);
		retval = (retval << 1) |
			 !!(getfrom_srom(priv, ioaddr) & EE_DATA_READ);
	} while (--cmd_len >= 0);

	sendto_srom(priv, SROM_RD | SROM_SR | DT_CS, ioaddr);

	/* Terminate the EEPROM access. */
	sendto_srom(priv, SROM_RD | SROM_SR, ioaddr);

	debug_cond(SROM_DLEVEL >= 1, " EEPROM result is 0x%5.5x.\n", retval);

	return retval;
}

static int read_srom(struct dc2114x_priv *priv, u_long ioaddr, int index)
{
	int ee_addr_size;

	ee_addr_size = (do_read_eeprom(priv, ioaddr, 0xff, 8) & BIT(18)) ? 8 : 6;

	return do_eeprom_cmd(priv, ioaddr, 0xffff |
			     (((SROM_READ_CMD << ee_addr_size) | index) << 16),
			     3 + ee_addr_size + 16);
}

static void send_setup_frame(struct dc2114x_priv *priv)
{
	char setup_frame[SETUP_FRAME_LEN];
	char *pa = &setup_frame[0];
	int i;

	memset(pa, 0xff, SETUP_FRAME_LEN);

	for (i = 0; i < ETH_ALEN; i++) {
		*(pa + (i & 1)) = priv->enetaddr[i];
		if (i & 0x01)
			pa += 4;
	}

	for (i = 0; priv->tx_ring[priv->tx_new].status & cpu_to_le32(T_OWN); i++) {
		if (i < TOUT_LOOP)
			continue;

		printf("%s: tx error buffer not ready\n", priv->name);
		return;
	}

	priv->tx_ring[priv->tx_new].buf = cpu_to_le32(phys_to_bus(priv->devno,
						      (u32)&setup_frame[0]));
	priv->tx_ring[priv->tx_new].des1 = cpu_to_le32(TD_TER | TD_SET | SETUP_FRAME_LEN);
	priv->tx_ring[priv->tx_new].status = cpu_to_le32(T_OWN);

	dc2114x_outl(priv, POLL_DEMAND, DE4X5_TPD);

	for (i = 0; priv->tx_ring[priv->tx_new].status & cpu_to_le32(T_OWN); i++) {
		if (i < TOUT_LOOP)
			continue;

		printf("%s: tx buffer not ready\n", priv->name);
		return;
	}

	if (le32_to_cpu(priv->tx_ring[priv->tx_new].status) != 0x7FFFFFFF) {
		printf("TX error status2 = 0x%08X\n",
		       le32_to_cpu(priv->tx_ring[priv->tx_new].status));
	}

	priv->tx_new = (priv->tx_new + 1) % NUM_TX_DESC;
}

static int dc21x4x_send_common(struct dc2114x_priv *priv, void *packet, int length)
{
	int status = -1;
	int i;

	if (length <= 0) {
		printf("%s: bad packet size: %d\n", priv->name, length);
		goto done;
	}

	for (i = 0; priv->tx_ring[priv->tx_new].status & cpu_to_le32(T_OWN); i++) {
		if (i < TOUT_LOOP)
			continue;

		printf("%s: tx error buffer not ready\n", priv->name);
		goto done;
	}

	priv->tx_ring[priv->tx_new].buf = cpu_to_le32(phys_to_bus(priv->devno,
						      (u32)packet));
	priv->tx_ring[priv->tx_new].des1 = cpu_to_le32(TD_TER | TD_LS | TD_FS | length);
	priv->tx_ring[priv->tx_new].status = cpu_to_le32(T_OWN);

	dc2114x_outl(priv, POLL_DEMAND, DE4X5_TPD);

	for (i = 0; priv->tx_ring[priv->tx_new].status & cpu_to_le32(T_OWN); i++) {
		if (i < TOUT_LOOP)
			continue;

		printf(".%s: tx buffer not ready\n", priv->name);
		goto done;
	}

	if (le32_to_cpu(priv->tx_ring[priv->tx_new].status) & TD_ES) {
		priv->tx_ring[priv->tx_new].status = 0x0;
		goto done;
	}

	status = length;

done:
	priv->tx_new = (priv->tx_new + 1) % NUM_TX_DESC;
	return status;
}

static int dc21x4x_recv_check(struct dc2114x_priv *priv)
{
	int length = 0;
	u32 status;

	status = le32_to_cpu(priv->rx_ring[priv->rx_new].status);

	if (status & R_OWN)
		return 0;

	if (status & RD_LS) {
		/* Valid frame status. */
		if (status & RD_ES) {
			/* There was an error. */
			printf("RX error status = 0x%08X\n", status);
			return -EINVAL;
		} else {
			/* A valid frame received. */
			length = (le32_to_cpu(priv->rx_ring[priv->rx_new].status)
				  >> 16);

			return length;
		}
	}

	return -EAGAIN;
}

static int dc21x4x_init_common(struct dc2114x_priv *priv)
{
	int i;

	reset_de4x5(priv);

	if (dc2114x_inl(priv, DE4X5_STS) & (STS_TS | STS_RS)) {
		printf("Error: Cannot reset ethernet controller.\n");
		return -1;
	}

	dc2114x_outl(priv, OMR_SDP | OMR_PS | OMR_PM, DE4X5_OMR);

	for (i = 0; i < NUM_RX_DESC; i++) {
		priv->rx_ring[i].status = cpu_to_le32(R_OWN);
		priv->rx_ring[i].des1 = cpu_to_le32(RX_BUFF_SZ);
		priv->rx_ring[i].buf = cpu_to_le32(phys_to_bus(priv->devno,
					     (u32)net_rx_packets[i]));
		priv->rx_ring[i].next = 0;
	}

	for (i = 0; i < NUM_TX_DESC; i++) {
		priv->tx_ring[i].status = 0;
		priv->tx_ring[i].des1 = 0;
		priv->tx_ring[i].buf = 0;
		priv->tx_ring[i].next = 0;
	}

	priv->rx_ring_size = NUM_RX_DESC;
	priv->tx_ring_size = NUM_TX_DESC;

	/* Write the end of list marker to the descriptor lists. */
	priv->rx_ring[priv->rx_ring_size - 1].des1 |= cpu_to_le32(RD_RER);
	priv->tx_ring[priv->tx_ring_size - 1].des1 |= cpu_to_le32(TD_TER);

	/* Tell the adapter where the TX/RX rings are located. */
	dc2114x_outl(priv, phys_to_bus(priv->devno, (u32)&priv->rx_ring),
		     DE4X5_RRBA);
	dc2114x_outl(priv, phys_to_bus(priv->devno, (u32)&priv->tx_ring),
		     DE4X5_TRBA);

	start_de4x5(priv);

	priv->tx_new = 0;
	priv->rx_new = 0;

	send_setup_frame(priv);

	return 0;
}

static void dc21x4x_halt_common(struct dc2114x_priv *priv)
{
	stop_de4x5(priv);
	dc2114x_outl(priv, 0, DE4X5_SICR);
}

static void read_hw_addr(struct dc2114x_priv *priv)
{
	u_short tmp, *p = (u_short *)(&priv->enetaddr[0]);
	int i, j = 0;

	for (i = 0; i < (ETH_ALEN >> 1); i++) {
		tmp = read_srom(priv, DE4X5_APROM, (SROM_HWADD >> 1) + i);
		*p = le16_to_cpu(tmp);
		j += *p++;
	}

	if (!j || j == 0x2fffd) {
		memset(priv->enetaddr, 0, ETH_ALEN);
		debug("Warning: can't read HW address from SROM.\n");
	}
}

static struct pci_device_id supported[] = {
	{ PCI_DEVICE(PCI_VENDOR_ID_DEC, PCI_DEVICE_ID_DEC_TULIP_FAST) },
	{ PCI_DEVICE(PCI_VENDOR_ID_DEC, PCI_DEVICE_ID_DEC_21142) },
	{ }
};

static int dc2114x_start(struct udevice *dev)
{
	struct eth_pdata *plat = dev_get_plat(dev);
	struct dc2114x_priv *priv = dev_get_priv(dev);

	memcpy(priv->enetaddr, plat->enetaddr, sizeof(plat->enetaddr));

	/* Ensure we're not sleeping. */
	dm_pci_write_config8(dev, PCI_CFDA_PSM, WAKEUP);

	return dc21x4x_init_common(priv);
}

static void dc2114x_stop(struct udevice *dev)
{
	struct dc2114x_priv *priv = dev_get_priv(dev);

	dc21x4x_halt_common(priv);

	dm_pci_write_config8(dev, PCI_CFDA_PSM, SLEEP);
}

static int dc2114x_send(struct udevice *dev, void *packet, int length)
{
	struct dc2114x_priv *priv = dev_get_priv(dev);
	int ret;

	ret = dc21x4x_send_common(priv, packet, length);

	return ret ? 0 : -ETIMEDOUT;
}

static int dc2114x_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct dc2114x_priv *priv = dev_get_priv(dev);
	int ret;

	ret = dc21x4x_recv_check(priv);

	if (ret < 0) {
		/* Update entry information. */
		priv->rx_new = (priv->rx_new + 1) % priv->rx_ring_size;
		ret = 0;
	}

	if (!ret)
		return 0;

	*packetp = net_rx_packets[priv->rx_new];

	return ret - 4;
}

static int dc2114x_free_pkt(struct udevice *dev, uchar *packet, int length)
{
	struct dc2114x_priv *priv = dev_get_priv(dev);

	priv->rx_ring[priv->rx_new].status = cpu_to_le32(R_OWN);

	/* Update entry information. */
	priv->rx_new = (priv->rx_new + 1) % priv->rx_ring_size;

	return 0;
}

static int dc2114x_read_rom_hwaddr(struct udevice *dev)
{
	struct dc2114x_priv *priv = dev_get_priv(dev);

	read_hw_addr(priv);

	return 0;
}

static int dc2114x_bind(struct udevice *dev)
{
	static int card_number;
	char name[16];

	sprintf(name, "dc2114x#%u", card_number++);

	return device_set_name(dev, name);
}

static int dc2114x_probe(struct udevice *dev)
{
	struct eth_pdata *plat = dev_get_plat(dev);
	struct dc2114x_priv *priv = dev_get_priv(dev);
	u16 command, status;
	u32 iobase;

	dm_pci_read_config32(dev, PCI_BASE_ADDRESS_1, &iobase);
	iobase &= ~0xf;

	debug("dc2114x: DEC 2114x PCI Device @0x%x\n", iobase);

	priv->devno = dev;
	priv->enetaddr = plat->enetaddr;
	priv->iobase = (void __iomem *)dm_pci_mem_to_phys(dev, iobase);

	command = PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER;
	dm_pci_write_config16(dev, PCI_COMMAND, command);
	dm_pci_read_config16(dev, PCI_COMMAND, &status);
	if ((status & command) != command) {
		printf("dc2114x: Couldn't enable IO access or Bus Mastering\n");
		return -EINVAL;
	}

	dm_pci_write_config8(dev, PCI_LATENCY_TIMER, 0x60);

	return 0;
}

static const struct eth_ops dc2114x_ops = {
	.start		= dc2114x_start,
	.send		= dc2114x_send,
	.recv		= dc2114x_recv,
	.stop		= dc2114x_stop,
	.free_pkt	= dc2114x_free_pkt,
	.read_rom_hwaddr = dc2114x_read_rom_hwaddr,
};

U_BOOT_DRIVER(eth_dc2114x) = {
	.name	= "eth_dc2114x",
	.id	= UCLASS_ETH,
	.bind	= dc2114x_bind,
	.probe	= dc2114x_probe,
	.ops	= &dc2114x_ops,
	.priv_auto	= sizeof(struct dc2114x_priv),
	.plat_auto	= sizeof(struct eth_pdata),
};

U_BOOT_PCI_DEVICE(eth_dc2114x, supported);
