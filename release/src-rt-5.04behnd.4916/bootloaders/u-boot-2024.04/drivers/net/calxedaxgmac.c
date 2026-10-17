// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2010-2011 Calxeda, Inc.
 */

#include <common.h>
#include <malloc.h>
#include <net.h>
#include <linux/compiler.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <asm/io.h>
#include <dm.h>
#include <dm/device-internal.h>	/* for dev_set_priv() */

#define TX_NUM_DESC			1
#define RX_NUM_DESC			32

#define MAC_TIMEOUT			(5*CONFIG_SYS_HZ)

#define ETH_BUF_SZ			2048
#define TX_BUF_SZ			(ETH_BUF_SZ * TX_NUM_DESC)
#define RX_BUF_SZ			(ETH_BUF_SZ * RX_NUM_DESC)

#define RXSTART				0x00000002
#define TXSTART				0x00002000

#define RXENABLE			0x00000004
#define TXENABLE			0x00000008

#define XGMAC_CONTROL_SPD		0x40000000
#define XGMAC_CONTROL_SPD_MASK		0x60000000
#define XGMAC_CONTROL_SARC		0x10000000
#define XGMAC_CONTROL_SARK_MASK		0x18000000
#define XGMAC_CONTROL_CAR		0x04000000
#define XGMAC_CONTROL_CAR_MASK		0x06000000
#define XGMAC_CONTROL_CAR_SHIFT		25
#define XGMAC_CONTROL_DP		0x01000000
#define XGMAC_CONTROL_WD		0x00800000
#define XGMAC_CONTROL_JD		0x00400000
#define XGMAC_CONTROL_JE		0x00100000
#define XGMAC_CONTROL_LM		0x00001000
#define XGMAC_CONTROL_IPC		0x00000400
#define XGMAC_CONTROL_ACS		0x00000080
#define XGMAC_CONTROL_DDIC		0x00000010
#define XGMAC_CONTROL_TE		0x00000008
#define XGMAC_CONTROL_RE		0x00000004

#define XGMAC_DMA_BUSMODE_RESET		0x00000001
#define XGMAC_DMA_BUSMODE_DSL		0x00000004
#define XGMAC_DMA_BUSMODE_DSL_MASK	0x0000007c
#define XGMAC_DMA_BUSMODE_DSL_SHIFT	2
#define XGMAC_DMA_BUSMODE_ATDS		0x00000080
#define XGMAC_DMA_BUSMODE_PBL_MASK	0x00003f00
#define XGMAC_DMA_BUSMODE_PBL_SHIFT	8
#define XGMAC_DMA_BUSMODE_FB		0x00010000
#define XGMAC_DMA_BUSMODE_USP		0x00800000
#define XGMAC_DMA_BUSMODE_8PBL		0x01000000
#define XGMAC_DMA_BUSMODE_AAL		0x02000000

#define XGMAC_DMA_AXIMODE_ENLPI		0x80000000
#define XGMAC_DMA_AXIMODE_MGK		0x40000000
#define XGMAC_DMA_AXIMODE_WROSR		0x00100000
#define XGMAC_DMA_AXIMODE_WROSR_MASK	0x00F00000
#define XGMAC_DMA_AXIMODE_WROSR_SHIFT	20
#define XGMAC_DMA_AXIMODE_RDOSR		0x00010000
#define XGMAC_DMA_AXIMODE_RDOSR_MASK	0x000F0000
#define XGMAC_DMA_AXIMODE_RDOSR_SHIFT	16
#define XGMAC_DMA_AXIMODE_AAL		0x00001000
#define XGMAC_DMA_AXIMODE_BLEN256	0x00000080
#define XGMAC_DMA_AXIMODE_BLEN128	0x00000040
#define XGMAC_DMA_AXIMODE_BLEN64	0x00000020
#define XGMAC_DMA_AXIMODE_BLEN32	0x00000010
#define XGMAC_DMA_AXIMODE_BLEN16	0x00000008
#define XGMAC_DMA_AXIMODE_BLEN8		0x00000004
#define XGMAC_DMA_AXIMODE_BLEN4		0x00000002
#define XGMAC_DMA_AXIMODE_UNDEF		0x00000001

#define XGMAC_CORE_OMR_RTC_SHIFT	3
#define XGMAC_CORE_OMR_RTC_MASK		0x00000018
#define XGMAC_CORE_OMR_RTC		0x00000010
#define XGMAC_CORE_OMR_RSF		0x00000020
#define XGMAC_CORE_OMR_DT		0x00000040
#define XGMAC_CORE_OMR_FEF		0x00000080
#define XGMAC_CORE_OMR_EFC		0x00000100
#define XGMAC_CORE_OMR_RFA_SHIFT	9
#define XGMAC_CORE_OMR_RFA_MASK		0x00000E00
#define XGMAC_CORE_OMR_RFD_SHIFT	12
#define XGMAC_CORE_OMR_RFD_MASK		0x00007000
#define XGMAC_CORE_OMR_TTC_SHIFT	16
#define XGMAC_CORE_OMR_TTC_MASK		0x00030000
#define XGMAC_CORE_OMR_TTC		0x00020000
#define XGMAC_CORE_OMR_FTF		0x00100000
#define XGMAC_CORE_OMR_TSF		0x00200000

#define FIFO_MINUS_1K			0x0
#define FIFO_MINUS_2K			0x1
#define FIFO_MINUS_3K			0x2
#define FIFO_MINUS_4K			0x3
#define FIFO_MINUS_6K			0x4
#define FIFO_MINUS_8K			0x5
#define FIFO_MINUS_12K			0x6
#define FIFO_MINUS_16K			0x7

#define XGMAC_CORE_FLOW_PT_SHIFT	16
#define XGMAC_CORE_FLOW_PT_MASK		0xFFFF0000
#define XGMAC_CORE_FLOW_PT		0x00010000
#define XGMAC_CORE_FLOW_DZQP		0x00000080
#define XGMAC_CORE_FLOW_PLT_SHIFT	4
#define XGMAC_CORE_FLOW_PLT_MASK	0x00000030
#define XGMAC_CORE_FLOW_PLT		0x00000010
#define XGMAC_CORE_FLOW_UP		0x00000008
#define XGMAC_CORE_FLOW_RFE		0x00000004
#define XGMAC_CORE_FLOW_TFE		0x00000002
#define XGMAC_CORE_FLOW_FCB		0x00000001

/* XGMAC Descriptor Defines */
#define MAX_DESC_BUF_SZ			(0x2000 - 8)

#define RXDESC_EXT_STATUS		0x00000001
#define RXDESC_CRC_ERR			0x00000002
#define RXDESC_RX_ERR			0x00000008
#define RXDESC_RX_WDOG			0x00000010
#define RXDESC_FRAME_TYPE		0x00000020
#define RXDESC_GIANT_FRAME		0x00000080
#define RXDESC_LAST_SEG			0x00000100
#define RXDESC_FIRST_SEG		0x00000200
#define RXDESC_VLAN_FRAME		0x00000400
#define RXDESC_OVERFLOW_ERR		0x00000800
#define RXDESC_LENGTH_ERR		0x00001000
#define RXDESC_SA_FILTER_FAIL		0x00002000
#define RXDESC_DESCRIPTOR_ERR		0x00004000
#define RXDESC_ERROR_SUMMARY		0x00008000
#define RXDESC_FRAME_LEN_OFFSET		16
#define RXDESC_FRAME_LEN_MASK		0x3fff0000
#define RXDESC_DA_FILTER_FAIL		0x40000000

#define RXDESC1_END_RING		0x00008000

#define RXDESC_IP_PAYLOAD_MASK		0x00000003
#define RXDESC_IP_PAYLOAD_UDP		0x00000001
#define RXDESC_IP_PAYLOAD_TCP		0x00000002
#define RXDESC_IP_PAYLOAD_ICMP		0x00000003
#define RXDESC_IP_HEADER_ERR		0x00000008
#define RXDESC_IP_PAYLOAD_ERR		0x00000010
#define RXDESC_IPV4_PACKET		0x00000040
#define RXDESC_IPV6_PACKET		0x00000080
#define TXDESC_UNDERFLOW_ERR		0x00000001
#define TXDESC_JABBER_TIMEOUT		0x00000002
#define TXDESC_LOCAL_FAULT		0x00000004
#define TXDESC_REMOTE_FAULT		0x00000008
#define TXDESC_VLAN_FRAME		0x00000010
#define TXDESC_FRAME_FLUSHED		0x00000020
#define TXDESC_IP_HEADER_ERR		0x00000040
#define TXDESC_PAYLOAD_CSUM_ERR		0x00000080
#define TXDESC_ERROR_SUMMARY		0x00008000
#define TXDESC_SA_CTRL_INSERT		0x00040000
#define TXDESC_SA_CTRL_REPLACE		0x00080000
#define TXDESC_2ND_ADDR_CHAINED		0x00100000
#define TXDESC_END_RING			0x00200000
#define TXDESC_CSUM_IP			0x00400000
#define TXDESC_CSUM_IP_PAYLD		0x00800000
#define TXDESC_CSUM_ALL			0x00C00000
#define TXDESC_CRC_EN_REPLACE		0x01000000
#define TXDESC_CRC_EN_APPEND		0x02000000
#define TXDESC_DISABLE_PAD		0x04000000
#define TXDESC_FIRST_SEG		0x10000000
#define TXDESC_LAST_SEG			0x20000000
#define TXDESC_INTERRUPT		0x40000000

#define DESC_OWN			0x80000000
#define DESC_BUFFER1_SZ_MASK		0x00001fff
#define DESC_BUFFER2_SZ_MASK		0x1fff0000
#define DESC_BUFFER2_SZ_OFFSET		16

struct xgmac_regs {
	u32 config;
	u32 framefilter;
	u32 resv_1[4];
	u32 flow_control;
	u32 vlantag;
	u32 version;
	u32 vlaninclude;
	u32 resv_2[2];
	u32 pacestretch;
	u32 vlanhash;
	u32 resv_3;
	u32 intreg;
	struct {
		u32 hi;         /* 0x40 */
		u32 lo;         /* 0x44 */
	} macaddr[16];
	u32 resv_4[0xd0];
	u32 core_opmode;	/* 0x400 */
	u32 resv_5[0x2bf];
	u32 busmode;		/* 0xf00 */
	u32 txpoll;
	u32 rxpoll;
	u32 rxdesclist;
	u32 txdesclist;
	u32 dma_status;
	u32 dma_opmode;
	u32 intenable;
	u32 resv_6[2];
	u32 axi_mode;		/* 0xf28 */
};

struct xgmac_dma_desc {
	__le32 flags;
	__le32 buf_size;
	__le32 buf1_addr;		/* Buffer 1 Address Pointer */
	__le32 buf2_addr;		/* Buffer 2 Address Pointer */
	__le32 ext_status;
	__le32 res[3];
};

static struct xgmac_regs *xgmac_get_regs(struct eth_pdata *pdata)
{
	/*
	 * We use PHYS_64BIT on Highbank, so phys_addr_t is bigger than
	 * a pointer. U-Boot doesn't use LPAE (not even the MMU on highbank),
	 * so we can't access anything above 4GB.
	 * We have a check in the probe function below the ensure this,
	 * so casting to a 32-bit pointer type is fine here.
	 */
	return (struct xgmac_regs *)(uintptr_t)pdata->iobase;
}

/* XGMAC Descriptor Access Helpers */
static inline void desc_set_buf_len(struct xgmac_dma_desc *p, u32 buf_sz)
{
	if (buf_sz > MAX_DESC_BUF_SZ)
		p->buf_size = cpu_to_le32(MAX_DESC_BUF_SZ |
			(buf_sz - MAX_DESC_BUF_SZ) << DESC_BUFFER2_SZ_OFFSET);
	else
		p->buf_size = cpu_to_le32(buf_sz);
}

static inline int desc_get_buf_len(struct xgmac_dma_desc *p)
{
	u32 len = le32_to_cpu(p->buf_size);
	return (len & DESC_BUFFER1_SZ_MASK) +
		((len & DESC_BUFFER2_SZ_MASK) >> DESC_BUFFER2_SZ_OFFSET);
}

static inline void desc_init_rx_desc(struct xgmac_dma_desc *p, int ring_size,
				     int buf_sz)
{
	struct xgmac_dma_desc *end = p + ring_size - 1;

	memset(p, 0, sizeof(*p) * ring_size);

	for (; p <= end; p++)
		desc_set_buf_len(p, buf_sz);

	end->buf_size |= cpu_to_le32(RXDESC1_END_RING);
}

static inline void desc_init_tx_desc(struct xgmac_dma_desc *p, u32 ring_size)
{
	memset(p, 0, sizeof(*p) * ring_size);
	p[ring_size - 1].flags = cpu_to_le32(TXDESC_END_RING);
}

static inline int desc_get_owner(struct xgmac_dma_desc *p)
{
	return le32_to_cpu(p->flags) & DESC_OWN;
}

static inline void desc_set_rx_owner(struct xgmac_dma_desc *p)
{
	/* Clear all fields and set the owner */
	p->flags = cpu_to_le32(DESC_OWN);
}

static inline void desc_set_tx_owner(struct xgmac_dma_desc *p, u32 flags)
{
	u32 tmpflags = le32_to_cpu(p->flags);
	tmpflags &= TXDESC_END_RING;
	tmpflags |= flags | DESC_OWN;
	p->flags = cpu_to_le32(tmpflags);
}

static inline void *desc_get_buf_addr(struct xgmac_dma_desc *p)
{
	return (void *)le32_to_cpu(p->buf1_addr);
}

static inline void desc_set_buf_addr(struct xgmac_dma_desc *p,
				     void *paddr, int len)
{
	p->buf1_addr = cpu_to_le32(paddr);
	if (len > MAX_DESC_BUF_SZ)
		p->buf2_addr = cpu_to_le32(paddr + MAX_DESC_BUF_SZ);
}

static inline void desc_set_buf_addr_and_size(struct xgmac_dma_desc *p,
					      void *paddr, int len)
{
	desc_set_buf_len(p, len);
	desc_set_buf_addr(p, paddr, len);
}

static inline int desc_get_rx_frame_len(struct xgmac_dma_desc *p)
{
	u32 data = le32_to_cpu(p->flags);
	u32 len = (data & RXDESC_FRAME_LEN_MASK) >> RXDESC_FRAME_LEN_OFFSET;
	if (data & RXDESC_FRAME_TYPE)
		len -= 4;

	return len;
}

struct calxeda_eth_dev {
	struct xgmac_dma_desc rx_chain[RX_NUM_DESC];
	struct xgmac_dma_desc tx_chain[TX_NUM_DESC];
	char rxbuffer[RX_BUF_SZ];

	u32 tx_currdesc;
	u32 rx_currdesc;
} __aligned(32);

/*
 * Initialize a descriptor ring.  Calxeda XGMAC is configured to use
 * advanced descriptors.
 */

static void init_rx_desc(struct eth_pdata *pdata, struct calxeda_eth_dev *priv)
{
	struct xgmac_dma_desc *rxdesc = priv->rx_chain;
	struct xgmac_regs *regs = xgmac_get_regs(pdata);
	void *rxbuffer = priv->rxbuffer;
	int i;

	desc_init_rx_desc(rxdesc, RX_NUM_DESC, ETH_BUF_SZ);
	writel((ulong)rxdesc, &regs->rxdesclist);

	for (i = 0; i < RX_NUM_DESC; i++) {
		desc_set_buf_addr(rxdesc + i, rxbuffer + (i * ETH_BUF_SZ),
				  ETH_BUF_SZ);
		desc_set_rx_owner(rxdesc + i);
	}
}

static void init_tx_desc(struct eth_pdata *pdata, struct calxeda_eth_dev *priv)
{
	struct xgmac_regs *regs = xgmac_get_regs(pdata);

	desc_init_tx_desc(priv->tx_chain, TX_NUM_DESC);
	writel((ulong)priv->tx_chain, &regs->txdesclist);
}

static int xgmac_reset(struct xgmac_regs *regs)
{
	int timeout = MAC_TIMEOUT;
	u32 value;

	value = readl(&regs->config) & XGMAC_CONTROL_SPD_MASK;

	writel(XGMAC_DMA_BUSMODE_RESET, &regs->busmode);
	while ((timeout-- >= 0) &&
		(readl(&regs->busmode) & XGMAC_DMA_BUSMODE_RESET))
		udelay(1);

	writel(value, &regs->config);

	return timeout;
}

static void xgmac_hwmacaddr(struct eth_pdata *pdata)
{
	struct xgmac_regs *regs = xgmac_get_regs(pdata);
	u32 macaddr[2];

	memcpy(macaddr, pdata->enetaddr, ARP_HLEN);
	writel(macaddr[1], &regs->macaddr[0].hi);
	writel(macaddr[0], &regs->macaddr[0].lo);
}

static int xgmac_eth_start(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct xgmac_regs *regs = xgmac_get_regs(pdata);
	struct calxeda_eth_dev *priv = dev_get_priv(dev);
	u32 value;

	if (xgmac_reset(regs) < 0)
		return -ETIMEDOUT;

	/* set the hardware MAC address */
	xgmac_hwmacaddr(pdata);

	/* set the AXI bus modes */
	value = XGMAC_DMA_BUSMODE_ATDS |
		(16 << XGMAC_DMA_BUSMODE_PBL_SHIFT) |
		XGMAC_DMA_BUSMODE_FB | XGMAC_DMA_BUSMODE_AAL;
	writel(value, &regs->busmode);

	value = XGMAC_DMA_AXIMODE_AAL | XGMAC_DMA_AXIMODE_BLEN16 |
		XGMAC_DMA_AXIMODE_BLEN8 | XGMAC_DMA_AXIMODE_BLEN4;
	writel(value, &regs->axi_mode);

	/* set flow control parameters and store and forward mode */
	value = (FIFO_MINUS_12K << XGMAC_CORE_OMR_RFD_SHIFT) |
		(FIFO_MINUS_4K << XGMAC_CORE_OMR_RFA_SHIFT) |
		XGMAC_CORE_OMR_EFC | XGMAC_CORE_OMR_TSF;
	writel(value, &regs->core_opmode);

	/* enable pause frames */
	value = (1024 << XGMAC_CORE_FLOW_PT_SHIFT) |
		(1 << XGMAC_CORE_FLOW_PLT_SHIFT) |
		XGMAC_CORE_FLOW_UP | XGMAC_CORE_FLOW_RFE | XGMAC_CORE_FLOW_TFE;
	writel(value, &regs->flow_control);

	/* Initialize the descriptor chains */
	init_rx_desc(pdata, priv);
	init_tx_desc(pdata, priv);

	/* must set to 0, or when started up will cause issues */
	priv->tx_currdesc = 0;
	priv->rx_currdesc = 0;

	/* set default core values */
	value = readl(&regs->config);
	value &= XGMAC_CONTROL_SPD_MASK;
	value |= XGMAC_CONTROL_DDIC | XGMAC_CONTROL_ACS |
		XGMAC_CONTROL_IPC | XGMAC_CONTROL_CAR;

	/* Everything is ready enable both mac and DMA */
	value |= RXENABLE | TXENABLE;
	writel(value, &regs->config);

	value = readl(&regs->dma_opmode);
	value |= RXSTART | TXSTART;
	writel(value, &regs->dma_opmode);

	return 0;
}

static int xgmac_tx(struct udevice *dev, void *packet, int length)
{
	struct calxeda_eth_dev *priv = dev_get_priv(dev);
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct xgmac_regs *regs = xgmac_get_regs(pdata);
	u32 currdesc = priv->tx_currdesc;
	struct xgmac_dma_desc *txdesc = &priv->tx_chain[currdesc];
	int timeout;

	desc_set_buf_addr_and_size(txdesc, packet, length);
	desc_set_tx_owner(txdesc, TXDESC_FIRST_SEG |
		TXDESC_LAST_SEG | TXDESC_CRC_EN_APPEND);

	/* write poll demand */
	writel(1, &regs->txpoll);

	timeout = 1000000;
	while (desc_get_owner(txdesc)) {
		if (timeout-- < 0) {
			printf("xgmac: TX timeout\n");
			return -ETIMEDOUT;
		}
		udelay(1);
	}

	priv->tx_currdesc = (currdesc + 1) & (TX_NUM_DESC - 1);
	return 0;
}

static int xgmac_rx(struct udevice *dev, int flags, uchar **packetp)
{
	struct calxeda_eth_dev *priv = dev_get_priv(dev);
	u32 currdesc = priv->rx_currdesc;
	struct xgmac_dma_desc *rxdesc = &priv->rx_chain[currdesc];
	int length = 0;

	/* check if the host has the desc */
	if (desc_get_owner(rxdesc))
		return -EAGAIN; /* the MAC is still chewing on it */

	length = desc_get_rx_frame_len(rxdesc);
	*packetp = desc_get_buf_addr(rxdesc);

	priv->rx_currdesc = (currdesc + 1) & (RX_NUM_DESC - 1);

	return length;
}

static int xgmac_free_pkt(struct udevice *dev, uchar *packet, int length)
{
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct xgmac_regs *regs = xgmac_get_regs(pdata);
	struct calxeda_eth_dev *priv = dev_get_priv(dev);
	u32 rxdesc = ((char *)packet - priv->rxbuffer) / ETH_BUF_SZ;
	struct xgmac_dma_desc *p = &priv->rx_chain[rxdesc];

	/* set descriptor back to owned by XGMAC */
	desc_set_rx_owner(p);
	writel(1, &regs->rxpoll);

	return 0;
}

static void xgmac_eth_stop(struct udevice *dev)
{
	struct calxeda_eth_dev *priv = dev_get_priv(dev);
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct xgmac_regs *regs = xgmac_get_regs(pdata);
	int value;

	/* Disable TX/RX */
	value = readl(&regs->config);
	value &= ~(RXENABLE | TXENABLE);
	writel(value, &regs->config);

	/* Disable DMA */
	value = readl(&regs->dma_opmode);
	value &= ~(RXSTART | TXSTART);
	writel(value, &regs->dma_opmode);

	/* must set to 0, or when started up will cause issues */
	priv->tx_currdesc = 0;
	priv->rx_currdesc = 0;
}

/*
 * Changing the MAC address is not a good idea, as the fabric would
 * need to know about this as well (it does not learn MAC addresses).
 */
static int xgmac_eth_write_hwaddr(struct udevice *dev)
{
	return -ENOSYS;
}

static int xgmac_eth_read_rom_hwaddr(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct xgmac_regs *regs = xgmac_get_regs(pdata);
	u32 macaddr[2];

	/* The MAC address is already configured, so read it from registers. */
	macaddr[1] = readl(&regs->macaddr[0].hi);
	macaddr[0] = readl(&regs->macaddr[0].lo);
	memcpy(pdata->enetaddr, macaddr, ARP_HLEN);

	return 0;
}

static int xgmac_ofdata_to_platdata(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct calxeda_eth_dev *priv;

	/* Structure must be aligned, because it contains the descriptors */
	priv = memalign(32, sizeof(*priv));
	if (!priv)
		return -ENOMEM;
	dev_set_priv(dev, priv);

	pdata->iobase = devfdt_get_addr(dev);
	if (pdata->iobase == FDT_ADDR_T_NONE) {
		printf("%s: Cannot find XGMAC base address\n", __func__);
		return -EINVAL;
	}
	if (pdata->iobase >= (1ULL << 32)) {
		printf("%s: MMIO base address cannot be above 4GB\n", __func__);
		return -EINVAL;
	}

	return 0;
}

static int xgmac_eth_probe(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct xgmac_regs *regs = xgmac_get_regs(pdata);

	/* check hardware version */
	if (readl(&regs->version) != 0x1012)
		return -ENODEV;

	xgmac_eth_read_rom_hwaddr(dev);

	return 0;
}

static const struct eth_ops xgmac_eth_ops = {
	.start		= xgmac_eth_start,
	.send		= xgmac_tx,
	.recv		= xgmac_rx,
	.free_pkt	= xgmac_free_pkt,
	.stop		= xgmac_eth_stop,
	.write_hwaddr	= xgmac_eth_write_hwaddr,
	.read_rom_hwaddr = xgmac_eth_read_rom_hwaddr,
};

static const struct udevice_id xgmac_eth_ids[] = {
	{ .compatible = "calxeda,hb-xgmac" },
	{ }
};

U_BOOT_DRIVER(eth_xgmac) = {
	.name		= "eth_xgmac",
	.id		= UCLASS_ETH,
	.of_match	= xgmac_eth_ids,
	.of_to_plat	= xgmac_ofdata_to_platdata,
	.probe		= xgmac_eth_probe,
	.ops		= &xgmac_eth_ops,
	.plat_auto	= sizeof(struct eth_pdata),
};
