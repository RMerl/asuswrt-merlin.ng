// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2002 Wolfgang Grandegger, wg@denx.de.
 *
 * This driver for AMD PCnet network controllers is derived from the
 * Linux driver pcnet32.c written 1996-1999 by Thomas Bogendoerfer.
 */

#include <common.h>
#include <cpu_func.h>
#include <dm.h>
#include <log.h>
#include <malloc.h>
#include <memalign.h>
#include <net.h>
#include <netdev.h>
#include <asm/cache.h>
#include <asm/io.h>
#include <pci.h>
#include <linux/delay.h>

#define	PCNET_DEBUG_LEVEL	0	/* 0=off, 1=init, 2=rx/tx */

#define PCNET_DEBUG1(fmt,args...)	\
	debug_cond(PCNET_DEBUG_LEVEL > 0, fmt ,##args)
#define PCNET_DEBUG2(fmt,args...)	\
	debug_cond(PCNET_DEBUG_LEVEL > 1, fmt ,##args)

/*
 * Set the number of Tx and Rx buffers, using Log_2(# buffers).
 * Reasonable default values are 4 Tx buffers, and 16 Rx buffers.
 * That translates to 2 (4 == 2^^2) and 4 (16 == 2^^4).
 */
#define PCNET_LOG_TX_BUFFERS	0
#define PCNET_LOG_RX_BUFFERS	2

#define TX_RING_SIZE		(1 << (PCNET_LOG_TX_BUFFERS))
#define TX_RING_LEN_BITS	((PCNET_LOG_TX_BUFFERS) << 12)

#define RX_RING_SIZE		(1 << (PCNET_LOG_RX_BUFFERS))
#define RX_RING_LEN_BITS	((PCNET_LOG_RX_BUFFERS) << 4)

#define PKT_BUF_SZ		1544

/* The PCNET Rx and Tx ring descriptors. */
struct pcnet_rx_head {
	u32 base;
	s16 buf_length;
	s16 status;
	u32 msg_length;
	u32 reserved;
};

struct pcnet_tx_head {
	u32 base;
	s16 length;
	s16 status;
	u32 misc;
	u32 reserved;
};

/* The PCNET 32-Bit initialization block, described in databook. */
struct pcnet_init_block {
	u16 mode;
	u16 tlen_rlen;
	u8 phys_addr[6];
	u16 reserved;
	u32 filter[2];
	/* Receive and transmit ring base, along with extra bits. */
	u32 rx_ring;
	u32 tx_ring;
	u32 reserved2;
};

struct pcnet_uncached_priv {
	struct pcnet_rx_head rx_ring[RX_RING_SIZE];
	struct pcnet_tx_head tx_ring[TX_RING_SIZE];
	struct pcnet_init_block init_block;
} __aligned(ARCH_DMA_MINALIGN);

struct pcnet_priv {
	struct pcnet_uncached_priv ucp;
	/* Receive Buffer space */
	unsigned char rx_buf[RX_RING_SIZE][PKT_BUF_SZ + 4];
	struct pcnet_uncached_priv *uc;
	struct udevice *dev;
	const char *name;
	void __iomem *iobase;
	u8 *enetaddr;
	u16 status;
	int cur_rx;
	int cur_tx;
};

/* Offsets from base I/O address for WIO mode */
#define PCNET_RDP		0x10
#define PCNET_RAP		0x12
#define PCNET_RESET		0x14
#define PCNET_BDP		0x16

static u16 pcnet_read_csr(struct pcnet_priv *lp, int index)
{
	writew(index, lp->iobase + PCNET_RAP);
	return readw(lp->iobase + PCNET_RDP);
}

static void pcnet_write_csr(struct pcnet_priv *lp, int index, u16 val)
{
	writew(index, lp->iobase + PCNET_RAP);
	writew(val, lp->iobase + PCNET_RDP);
}

static u16 pcnet_read_bcr(struct pcnet_priv *lp, int index)
{
	writew(index, lp->iobase + PCNET_RAP);
	return readw(lp->iobase + PCNET_BDP);
}

static void pcnet_write_bcr(struct pcnet_priv *lp, int index, u16 val)
{
	writew(index, lp->iobase + PCNET_RAP);
	writew(val, lp->iobase + PCNET_BDP);
}

static void pcnet_reset(struct pcnet_priv *lp)
{
	readw(lp->iobase + PCNET_RESET);
}

static int pcnet_check(struct pcnet_priv *lp)
{
	writew(88, lp->iobase + PCNET_RAP);
	return readw(lp->iobase + PCNET_RAP) == 88;
}

static inline pci_addr_t pcnet_virt_to_mem(struct pcnet_priv *lp, void *addr)
{
	void *virt_addr = addr;

	return dm_pci_virt_to_mem(lp->dev, virt_addr);
}

static struct pci_device_id supported[] = {
	{ PCI_DEVICE(PCI_VENDOR_ID_AMD, PCI_DEVICE_ID_AMD_LANCE) },
	{}
};

static int pcnet_probe_common(struct pcnet_priv *lp)
{
	int chip_version;
	char *chipname;
	int i;

	/* Reset the PCnet controller */
	pcnet_reset(lp);

	/* Check if register access is working */
	if (pcnet_read_csr(lp, 0) != 4 || !pcnet_check(lp)) {
		printf("%s: CSR register access check failed\n", lp->name);
		return -1;
	}

	/* Identify the chip */
	chip_version = pcnet_read_csr(lp, 88) | (pcnet_read_csr(lp, 89) << 16);
	if ((chip_version & 0xfff) != 0x003)
		return -1;
	chip_version = (chip_version >> 12) & 0xffff;
	switch (chip_version) {
	case 0x2621:
		chipname = "PCnet/PCI II 79C970A";	/* PCI */
		break;
	case 0x2625:
		chipname = "PCnet/FAST III 79C973";	/* PCI */
		break;
	case 0x2627:
		chipname = "PCnet/FAST III 79C975";	/* PCI */
		break;
	default:
		printf("%s: PCnet version %#x not supported\n",
		       lp->name, chip_version);
		return -1;
	}

	PCNET_DEBUG1("AMD %s\n", chipname);

	/*
	 * In most chips, after a chip reset, the ethernet address is read from
	 * the station address PROM at the base address and programmed into the
	 * "Physical Address Registers" CSR12-14.
	 */
	for (i = 0; i < 3; i++) {
		unsigned int val;

		val = pcnet_read_csr(lp, i + 12) & 0x0ffff;
		/* There may be endianness issues here. */
		lp->enetaddr[2 * i] = val & 0x0ff;
		lp->enetaddr[2 * i + 1] = (val >> 8) & 0x0ff;
	}

	return 0;
}

static int pcnet_init_common(struct pcnet_priv *lp)
{
	struct pcnet_uncached_priv *uc;
	int i, val;
	unsigned long addr;

	PCNET_DEBUG1("%s: %s...\n", lp->name, __func__);

	/* Switch pcnet to 32bit mode */
	pcnet_write_bcr(lp, 20, 2);

	/* Set/reset autoselect bit */
	val = pcnet_read_bcr(lp, 2) & ~2;
	val |= 2;
	pcnet_write_bcr(lp, 2, val);

	/* Enable auto negotiate, setup, disable fd */
	val = pcnet_read_bcr(lp, 32) & ~0x98;
	val |= 0x20;
	pcnet_write_bcr(lp, 32, val);

	/*
	 * Enable NOUFLO on supported controllers, with the transmit
	 * start point set to the full packet. This will cause entire
	 * packets to be buffered by the ethernet controller before
	 * transmission, eliminating underflows which are common on
	 * slower devices. Controllers which do not support NOUFLO will
	 * simply be left with a larger transmit FIFO threshold.
	 */
	val = pcnet_read_bcr(lp, 18);
	val |= 1 << 11;
	pcnet_write_bcr(lp, 18, val);
	val = pcnet_read_csr(lp, 80);
	val |= 0x3 << 10;
	pcnet_write_csr(lp, 80, val);

	uc = lp->uc;

	uc->init_block.mode = cpu_to_le16(0x0000);
	uc->init_block.filter[0] = 0x00000000;
	uc->init_block.filter[1] = 0x00000000;

	/*
	 * Initialize the Rx ring.
	 */
	lp->cur_rx = 0;
	for (i = 0; i < RX_RING_SIZE; i++) {
		addr = pcnet_virt_to_mem(lp, lp->rx_buf[i]);
		uc->rx_ring[i].base = cpu_to_le32(addr);
		uc->rx_ring[i].buf_length = cpu_to_le16(-PKT_BUF_SZ);
		uc->rx_ring[i].status = cpu_to_le16(0x8000);
		PCNET_DEBUG1
			("Rx%d: base=0x%x buf_length=0x%hx status=0x%hx\n", i,
			 uc->rx_ring[i].base, uc->rx_ring[i].buf_length,
			 uc->rx_ring[i].status);
	}

	/*
	 * Initialize the Tx ring. The Tx buffer address is filled in as
	 * needed, but we do need to clear the upper ownership bit.
	 */
	lp->cur_tx = 0;
	for (i = 0; i < TX_RING_SIZE; i++) {
		uc->tx_ring[i].base = 0;
		uc->tx_ring[i].status = 0;
	}

	/*
	 * Setup Init Block.
	 */
	PCNET_DEBUG1("Init block at 0x%p: MAC", &lp->uc->init_block);

	for (i = 0; i < 6; i++) {
		lp->uc->init_block.phys_addr[i] = lp->enetaddr[i];
		PCNET_DEBUG1(" %02x", lp->uc->init_block.phys_addr[i]);
	}

	uc->init_block.tlen_rlen = cpu_to_le16(TX_RING_LEN_BITS |
					       RX_RING_LEN_BITS);
	addr = pcnet_virt_to_mem(lp, uc->rx_ring);
	uc->init_block.rx_ring = cpu_to_le32(addr);
	addr = pcnet_virt_to_mem(lp, uc->tx_ring);
	uc->init_block.tx_ring = cpu_to_le32(addr);

	PCNET_DEBUG1("\ntlen_rlen=0x%x rx_ring=0x%x tx_ring=0x%x\n",
		     uc->init_block.tlen_rlen,
		     uc->init_block.rx_ring, uc->init_block.tx_ring);

	/*
	 * Tell the controller where the Init Block is located.
	 */
	barrier();
	addr = pcnet_virt_to_mem(lp, &lp->uc->init_block);
	pcnet_write_csr(lp, 1, addr & 0xffff);
	pcnet_write_csr(lp, 2, (addr >> 16) & 0xffff);

	pcnet_write_csr(lp, 4, 0x0915);
	pcnet_write_csr(lp, 0, 0x0001);	/* start */

	/* Wait for Init Done bit */
	for (i = 10000; i > 0; i--) {
		if (pcnet_read_csr(lp, 0) & 0x0100)
			break;
		udelay(10);
	}
	if (i <= 0) {
		printf("%s: TIMEOUT: controller init failed\n", lp->name);
		pcnet_reset(lp);
		return -1;
	}

	/*
	 * Finally start network controller operation.
	 */
	pcnet_write_csr(lp, 0, 0x0002);

	return 0;
}

static int pcnet_send_common(struct pcnet_priv *lp, void *packet, int pkt_len)
{
	int i, status;
	u32 addr;
	struct pcnet_tx_head *entry = &lp->uc->tx_ring[lp->cur_tx];

	PCNET_DEBUG2("Tx%d: %d bytes from 0x%p ", lp->cur_tx, pkt_len,
		     packet);

	flush_dcache_range((unsigned long)packet,
			   (unsigned long)packet + pkt_len);

	/* Wait for completion by testing the OWN bit */
	for (i = 1000; i > 0; i--) {
		status = readw(&entry->status);
		if ((status & 0x8000) == 0)
			break;
		udelay(100);
		PCNET_DEBUG2(".");
	}
	if (i <= 0) {
		printf("%s: TIMEOUT: Tx%d failed (status = 0x%x)\n",
		       lp->name, lp->cur_tx, status);
		pkt_len = 0;
		goto failure;
	}

	/*
	 * Setup Tx ring. Caution: the write order is important here,
	 * set the status with the "ownership" bits last.
	 */
	addr = pcnet_virt_to_mem(lp, packet);
	writew(-pkt_len, &entry->length);
	writel(0, &entry->misc);
	writel(addr, &entry->base);
	writew(0x8300, &entry->status);

	/* Trigger an immediate send poll. */
	pcnet_write_csr(lp, 0, 0x0008);

      failure:
	if (++lp->cur_tx >= TX_RING_SIZE)
		lp->cur_tx = 0;

	PCNET_DEBUG2("done\n");
	return pkt_len;
}

static int pcnet_recv_common(struct pcnet_priv *lp, unsigned char **bufp)
{
	struct pcnet_rx_head *entry;
	unsigned char *buf;
	int pkt_len = 0;
	u16 err_status;

	entry = &lp->uc->rx_ring[lp->cur_rx];
	/*
	 * If we own the next entry, it's a new packet. Send it up.
	 */
	lp->status = readw(&entry->status);
	if ((lp->status & 0x8000) != 0)
		return 0;
	err_status = lp->status >> 8;

	if (err_status != 0x03) {	/* There was an error. */
		printf("%s: Rx%d", lp->name, lp->cur_rx);
		PCNET_DEBUG1(" (status=0x%x)", err_status);
		if (err_status & 0x20)
			printf(" Frame");
		if (err_status & 0x10)
			printf(" Overflow");
		if (err_status & 0x08)
			printf(" CRC");
		if (err_status & 0x04)
			printf(" Fifo");
		printf(" Error\n");
		lp->status &= 0x03ff;
		return 0;
	}

	pkt_len = (readl(&entry->msg_length) & 0xfff) - 4;
	if (pkt_len < 60) {
		printf("%s: Rx%d: invalid packet length %d\n",
		       lp->name, lp->cur_rx, pkt_len);
		return 0;
	}

	*bufp = lp->rx_buf[lp->cur_rx];
	invalidate_dcache_range((unsigned long)*bufp,
				(unsigned long)*bufp + pkt_len);

	PCNET_DEBUG2("Rx%d: %d bytes from 0x%p\n",
		     lp->cur_rx, pkt_len, buf);

	return pkt_len;
}

static void pcnet_free_pkt_common(struct pcnet_priv *lp, unsigned int len)
{
	struct pcnet_rx_head *entry;

	entry = &lp->uc->rx_ring[lp->cur_rx];

	lp->status |= 0x8000;
	writew(lp->status, &entry->status);

	if (++lp->cur_rx >= RX_RING_SIZE)
		lp->cur_rx = 0;
}

static void pcnet_halt_common(struct pcnet_priv *lp)
{
	int i;

	PCNET_DEBUG1("%s: %s...\n", lp->name, __func__);

	/* Reset the PCnet controller */
	pcnet_reset(lp);

	/* Wait for Stop bit */
	for (i = 1000; i > 0; i--) {
		if (pcnet_read_csr(lp, 0) & 0x4)
			break;
		udelay(10);
	}
	if (i <= 0)
		printf("%s: TIMEOUT: controller reset failed\n", lp->name);
}

static int pcnet_start(struct udevice *dev)
{
	struct eth_pdata *plat = dev_get_plat(dev);
	struct pcnet_priv *priv = dev_get_priv(dev);

	memcpy(priv->enetaddr, plat->enetaddr, sizeof(plat->enetaddr));

	return pcnet_init_common(priv);
}

static void pcnet_stop(struct udevice *dev)
{
	struct pcnet_priv *priv = dev_get_priv(dev);

	pcnet_halt_common(priv);
}

static int pcnet_send(struct udevice *dev, void *packet, int length)
{
	struct pcnet_priv *priv = dev_get_priv(dev);
	int ret;

	ret = pcnet_send_common(priv, packet, length);

	return ret ? 0 : -ETIMEDOUT;
}

static int pcnet_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct pcnet_priv *priv = dev_get_priv(dev);

	return pcnet_recv_common(priv, packetp);
}

static int pcnet_free_pkt(struct udevice *dev, uchar *packet, int length)
{
	struct pcnet_priv *priv = dev_get_priv(dev);

	pcnet_free_pkt_common(priv, length);

	return 0;
}

static int pcnet_bind(struct udevice *dev)
{
	static int card_number;
	char name[16];

	sprintf(name, "pcnet#%u", card_number++);

	return device_set_name(dev, name);
}

static int pcnet_probe(struct udevice *dev)
{
	struct eth_pdata *plat = dev_get_plat(dev);
	struct pcnet_priv *lp = dev_get_priv(dev);
	u16 command, status;
	u32 iobase;
	int ret;

	dm_pci_read_config32(dev, PCI_BASE_ADDRESS_1, &iobase);
	iobase &= ~0xf;

	lp->uc = map_physmem((phys_addr_t)&lp->ucp,
			     sizeof(lp->ucp), MAP_NOCACHE);
	lp->dev = dev;
	lp->name = dev->name;
	lp->enetaddr = plat->enetaddr;
	lp->iobase = (void *)dm_pci_mem_to_phys(dev, iobase);

	flush_dcache_range((unsigned long)lp,
			   (unsigned long)lp + sizeof(*lp));

	command = PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER;
	dm_pci_write_config16(dev, PCI_COMMAND, command);
	dm_pci_read_config16(dev, PCI_COMMAND, &status);
	if ((status & command) != command) {
		printf("%s: Couldn't enable IO access or Bus Mastering\n",
		       lp->name);
		return -EINVAL;
	}

	dm_pci_write_config8(dev, PCI_LATENCY_TIMER, 0x20);

	ret = pcnet_probe_common(lp);
	if (ret)
		return ret;

	return 0;
}

static const struct eth_ops pcnet_ops = {
	.start		= pcnet_start,
	.send		= pcnet_send,
	.recv		= pcnet_recv,
	.stop		= pcnet_stop,
	.free_pkt	= pcnet_free_pkt,
};

U_BOOT_DRIVER(eth_pcnet) = {
	.name	= "eth_pcnet",
	.id	= UCLASS_ETH,
	.bind	= pcnet_bind,
	.probe	= pcnet_probe,
	.ops	= &pcnet_ops,
	.priv_auto	= sizeof(struct pcnet_priv),
	.plat_auto	= sizeof(struct eth_pdata),
	.flags	= DM_UC_FLAG_ALLOC_PRIV_DMA,
};

U_BOOT_PCI_DEVICE(eth_pcnet, supported);
