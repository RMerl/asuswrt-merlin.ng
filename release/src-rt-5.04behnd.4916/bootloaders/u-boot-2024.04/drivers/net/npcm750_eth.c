// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2021 Nuvoton Technology Corp.
 */

#include <common.h>
#include <cpu_func.h>
#include <dm.h>
#include <errno.h>
#include <miiphy.h>
#include <malloc.h>
#include <net.h>
#include <regmap.h>
#include <serial.h>
#include <syscon.h>
#include <asm/io.h>
#include <linux/err.h>
#include <linux/iopoll.h>

#define MAC_ADDR_SIZE		6
#define CFG_TX_DESCR_NUM	32
#define CFG_RX_DESCR_NUM	32

#define TX_TOTAL_BUFSIZE	\
		((CFG_TX_DESCR_NUM + 1) * PKTSIZE_ALIGN + PKTALIGN)
#define RX_TOTAL_BUFSIZE	\
		((CFG_RX_DESCR_NUM + 1) * PKTSIZE_ALIGN + PKTALIGN)

#define CFG_MDIO_TIMEOUT (3 * CONFIG_SYS_HZ)

struct npcm750_rxbd {
	unsigned int sl;
	unsigned int buffer;
	unsigned int reserved;
	unsigned int next;
} __aligned(ARCH_DMA_MINALIGN);

struct npcm750_txbd {
	unsigned int mode;
	unsigned int buffer;
	unsigned int sl;
	unsigned int next;
} __aligned(ARCH_DMA_MINALIGN);

struct emc_regs {
	u32 camcmr;		/* 0x00 */
	u32 camen;		/* 0x04 */
	u32 cam0m;		/* 0x08 */
	u32 cam0l;		/* 0x0c */
	u32 cam1m;		/* 0x10 */
	u32 cam1l;		/* 0x14 */
	u32 cam2m;		/* 0x18 */
	u32 cam2l;		/* 0x1c */
	u32 cam3m;		/* 0x20 */
	u32 cam3l;		/* 0x24 */
	u32 cam4m;		/* 0x28 */
	u32 cam4l;		/* 0x2c */
	u32 cam5m;		/* 0x30 */
	u32 cam5l;		/* 0x34 */
	u32 cam6m;		/* 0x38 */
	u32 cam6l;		/* 0x3c */
	u32 cam7m;		/* 0x40 */
	u32 cam7l;		/* 0x44 */
	u32 cam8m;		/* 0x48 */
	u32 cam8l;		/* 0x4c */
	u32 cam9m;		/* 0x50 */
	u32 cam9l;		/* 0x54 */
	u32 cam10m;		/* 0x58 */
	u32 cam10l;		/* 0x5c */
	u32 cam11m;		/* 0x60 */
	u32 cam11l;		/* 0x64 */
	u32 cam12m;		/* 0x68 */
	u32 cam12l;		/* 0x6c */
	u32 cam13m;		/* 0x70 */
	u32 cam13l;		/* 0x74 */
	u32 cam14m;		/* 0x78 */
	u32 cam14l;		/* 0x7c */
	u32 cam15m;		/* 0x80 */
	u32 cam15l;		/* 0x84 */
	u32 txdlsa;		/* 0x88 */
	u32 rxdlsa;		/* 0x8c */
	u32 mcmdr;		/* 0x90 */
	u32 miid;		/* 0x94 */
	u32 miida;		/* 0x98 */
	u32 fftcr;		/* 0x9c */
	u32 tsdr;		/* 0xa0 */
	u32 rsdr;		/* 0xa4 */
	u32 dmarfc;		/* 0xa8 */
	u32 mien;		/* 0xac */
	u32 mista;		/* 0xb0 */
	u32 mgsta;		/* 0xb4 */
	u32 mpcnt;		/* 0xb8 */
	u32 mrpc;		/* 0xbc */
	u32 mrpcc;		/* 0xc0 */
	u32 mrepc;		/* 0xc4 */
	u32 dmarfs;		/* 0xc8 */
	u32 ctxdsa;		/* 0xcc */
	u32 ctxbsa;		/* 0xd0 */
	u32 crxdsa;		/* 0xd4 */
	u32 crxbsa;		/* 0xd8 */
};

struct npcm750_eth_dev {
	struct npcm750_txbd tdesc[CFG_TX_DESCR_NUM] __aligned(ARCH_DMA_MINALIGN);
	struct npcm750_rxbd rdesc[CFG_RX_DESCR_NUM] __aligned(ARCH_DMA_MINALIGN);
	u8 txbuffs[TX_TOTAL_BUFSIZE] __aligned(ARCH_DMA_MINALIGN);
	u8 rxbuffs[RX_TOTAL_BUFSIZE] __aligned(ARCH_DMA_MINALIGN);
	struct emc_regs *emc_regs_p;
	struct phy_device *phydev;
	struct mii_dev *bus;
	struct npcm750_txbd *curr_txd;
	struct npcm750_rxbd *curr_rxd;
	u32 interface;
	u32 max_speed;
	u32 idx;
	struct regmap *gcr_regmap;
};

struct npcm750_eth_pdata {
	struct eth_pdata eth_pdata;
};

/* mac controller bit */
#define MCMDR_RXON		BIT(0)
#define MCMDR_ACP		BIT(3)
#define MCMDR_SPCRC		BIT(5)
#define MCMDR_TXON		BIT(8)
#define MCMDR_NDEF		BIT(9)
#define MCMDR_FDUP		BIT(18)
#define MCMDR_ENMDC		BIT(19)
#define MCMDR_OPMOD		BIT(20)
#define MCMDR_SWR		BIT(24)

/* cam command regiser */
#define CAMCMR_AUP		0x01
#define CAMCMR_AMP		BIT(1)
#define CAMCMR_ABP		BIT(2)
#define CAMCMR_CCAM		BIT(3)
#define CAMCMR_ECMP		BIT(4)
#define CAM0EN			0x01

/* mac mii controller bit */
#define MDCON			BIT(19)
#define PHYAD			BIT(8)
#define PHYWR			BIT(16)
#define PHYBUSY			BIT(17)
#define PHYPRESP		BIT(18)
#define CAM_ENTRY_SIZE	0x08

/* rx and tx status */
#define TXDS_TXCP		BIT(19)
#define RXDS_CRCE		BIT(17)
#define RXDS_PTLE		BIT(19)
#define RXDS_RXGD		BIT(20)
#define RXDS_ALIE		BIT(21)
#define RXDS_RP			BIT(22)

/* mac interrupt status*/
#define MISTA_RXINTR		BIT(0)
#define MISTA_CRCE		BIT(1)
#define MISTA_RXOV		BIT(2)
#define MISTA_PTLE		BIT(3)
#define MISTA_RXGD		BIT(4)
#define MISTA_ALIE		BIT(5)
#define MISTA_RP		BIT(6)
#define MISTA_MMP		BIT(7)
#define MISTA_DFOI		BIT(8)
#define MISTA_DENI		BIT(9)
#define MISTA_RDU		BIT(10)
#define MISTA_RXBERR		BIT(11)
#define MISTA_CFR		BIT(14)
#define MISTA_TXINTR		BIT(16)
#define MISTA_TXEMP		BIT(17)
#define MISTA_TXCP		BIT(18)
#define MISTA_EXDEF		BIT(19)
#define MISTA_NCS		BIT(20)
#define MISTA_TXABT		BIT(21)
#define MISTA_LC		BIT(22)
#define MISTA_TDU		BIT(23)
#define MISTA_TXBERR		BIT(24)

#define ENSTART			0x01
#define ENRXINTR		BIT(0)
#define ENCRCE			BIT(1)
#define EMRXOV			BIT(2)
#define ENPTLE			BIT(3)
#define ENRXGD			BIT(4)
#define ENALIE			BIT(5)
#define ENRP			BIT(6)
#define ENMMP			BIT(7)
#define ENDFO			BIT(8)
#define ENDENI			BIT(9)
#define ENRDU			BIT(10)
#define ENRXBERR		BIT(11)
#define ENCFR			BIT(14)
#define ENTXINTR		BIT(16)
#define ENTXEMP			BIT(17)
#define ENTXCP			BIT(18)
#define ENTXDEF			BIT(19)
#define ENNCS			BIT(20)
#define ENTXABT			BIT(21)
#define ENLC			BIT(22)
#define ENTDU			BIT(23)
#define ENTXBERR		BIT(24)

#define RX_STAT_RBC     0xffff
#define RX_STAT_RXINTR  BIT(16)
#define RX_STAT_CRCE    BIT(17)
#define RX_STAT_PTLE    BIT(19)
#define RX_STAT_RXGD    BIT(20)
#define RX_STAT_ALIE    BIT(21)
#define RX_STAT_RP      BIT(22)
#define RX_STAT_OWNER   (BIT(30) | BIT(31))

#define TX_STAT_TBC     0xffff
#define TX_STAT_TXINTR  BIT(16)
#define TX_STAT_DEF     BIT(17)
#define TX_STAT_TXCP    BIT(19)
#define TX_STAT_EXDEF   BIT(20)
#define TX_STAT_NCS     BIT(21)
#define TX_STAT_TXBT    BIT(22)
#define TX_STAT_LC      BIT(23)
#define TX_STAT_TXHA    BIT(24)
#define TX_STAT_PAU     BIT(25)
#define TX_STAT_SQE     BIT(26)

/* rx and tx owner bit */
#define RX_OWEN_DMA		BIT(31)
#define RX_OWEN_CPU		0x00       //bit 30 & bit 31
#define TX_OWEN_DMA		BIT(31)
#define TX_OWEN_CPU		(~(BIT(31)))

/* tx frame desc controller bit */
#define MACTXINTEN		0x04
#define CRCMODE			0x02
#define PADDINGMODE		0x01

/* fftcr controller bit */
#define RXTHD			0x03
#define TXTHD			(BIT(8) | BIT(9))
#define BLENGTH			BIT(21)

/* global setting for driver */
#define RX_DESC_SIZE	128
#define TX_DESC_SIZE	64
#define MAX_RBUFF_SZ	0x600
#define MAX_TBUFF_SZ	0x600
#define TX_TIMEOUT	50
#define DELAY		1000
#define CAM0		0x0
#define RX_POLL_SIZE	(RX_DESC_SIZE / 2)
#define MII_TIMEOUT	100
#define GCR_INTCR	0x3c
#define INTCR_R1EN	BIT(5)

enum MIIDA_MDCCR_T {
	MIIDA_MDCCR_4       = 0x00,
	MIIDA_MDCCR_6       = 0x01,
	MIIDA_MDCCR_8       = 0x02,
	MIIDA_MDCCR_12      = 0x03,
	MIIDA_MDCCR_16      = 0x04,
	MIIDA_MDCCR_20      = 0x05,
	MIIDA_MDCCR_24      = 0x06,
	MIIDA_MDCCR_28      = 0x07,
	MIIDA_MDCCR_30      = 0x08,
	MIIDA_MDCCR_32      = 0x09,
	MIIDA_MDCCR_36      = 0x0A,
	MIIDA_MDCCR_40      = 0x0B,
	MIIDA_MDCCR_44      = 0x0C,
	MIIDA_MDCCR_48      = 0x0D,
	MIIDA_MDCCR_54      = 0x0E,
	MIIDA_MDCCR_60      = 0x0F,
};

DECLARE_GLOBAL_DATA_PTR;

static int npcm750_mdio_read(struct mii_dev *bus, int addr, int devad, int regs)
{
	struct npcm750_eth_dev *priv = (struct npcm750_eth_dev *)bus->priv;
	struct emc_regs *reg = priv->emc_regs_p;
	u32 start, val;
	int timeout = CFG_MDIO_TIMEOUT;

	val = (addr << 0x08) | regs | PHYBUSY | (MIIDA_MDCCR_60 << 20);
	writel(val, &reg->miida);

	start = get_timer(0);
	while (get_timer(start) < timeout) {
		if (!(readl(&reg->miida) & PHYBUSY)) {
			val = readl(&reg->miid);
			return val;
		}
		udelay(10);
	};
	return -ETIMEDOUT;
}

static int npcm750_mdio_write(struct mii_dev *bus, int addr, int devad, int regs,
			      u16 val)
{
	struct npcm750_eth_dev *priv = (struct npcm750_eth_dev *)bus->priv;
	struct emc_regs *reg = priv->emc_regs_p;
	ulong start;
	int ret = -ETIMEDOUT, timeout = CFG_MDIO_TIMEOUT;

	writel(val, &reg->miid);
	writel((addr << 0x08) | regs | PHYBUSY | PHYWR | (MIIDA_MDCCR_60 << 20), &reg->miida);

	start = get_timer(0);
	while (get_timer(start) < timeout) {
		if (!(readl(&reg->miida) & PHYBUSY)) {
			ret = 0;
			break;
		}
		udelay(10);
	};
	return ret;
}

static int npcm750_mdio_reset(struct mii_dev *bus)
{
	return 0;
}

static int npcm750_mdio_init(const char *name, struct npcm750_eth_dev *priv)
{
	struct emc_regs *reg = priv->emc_regs_p;
	struct mii_dev *bus = mdio_alloc();

	if (!bus) {
		printf("Failed to allocate MDIO bus\n");
		return -ENOMEM;
	}

	bus->read = npcm750_mdio_read;
	bus->write = npcm750_mdio_write;
	snprintf(bus->name, sizeof(bus->name), "%s", name);
	bus->reset = npcm750_mdio_reset;

	bus->priv = (void *)priv;

	writel(readl(&reg->mcmdr) | MCMDR_ENMDC, &reg->mcmdr);
	return mdio_register(bus);
}

static void npcm750_tx_descs_init(struct npcm750_eth_dev *priv)
{
	struct emc_regs *reg = priv->emc_regs_p;
	struct npcm750_txbd *desc_table_p = &priv->tdesc[0];
	struct npcm750_txbd *desc_p;
	u8 *txbuffs = &priv->txbuffs[0];
	u32 idx;

	writel((u32)desc_table_p, &reg->txdlsa);
	priv->curr_txd = desc_table_p;

	for (idx = 0; idx < CFG_TX_DESCR_NUM; idx++) {
		desc_p = &desc_table_p[idx];
		desc_p->buffer = (u32)&txbuffs[idx * PKTSIZE_ALIGN];
		desc_p->sl = 0;
		desc_p->mode = 0;
		desc_p->mode = TX_OWEN_CPU | PADDINGMODE | CRCMODE | MACTXINTEN;
		if (idx < (CFG_TX_DESCR_NUM - 1))
			desc_p->next = (u32)&desc_table_p[idx + 1];
		else
			desc_p->next = (u32)&priv->tdesc[0];
	}
	flush_dcache_range((ulong)&desc_table_p[0],
			   (ulong)&desc_table_p[CFG_TX_DESCR_NUM]);
}

static void npcm750_rx_descs_init(struct npcm750_eth_dev *priv)
{
	struct emc_regs *reg = priv->emc_regs_p;
	struct npcm750_rxbd *desc_table_p = &priv->rdesc[0];
	struct npcm750_rxbd *desc_p;
	u8 *rxbuffs = &priv->rxbuffs[0];
	u32 idx;

	flush_dcache_range((ulong)priv->rxbuffs[0],
			   (ulong)priv->rxbuffs[CFG_RX_DESCR_NUM]);

	writel((u32)desc_table_p, &reg->rxdlsa);
	priv->curr_rxd = desc_table_p;

	for (idx = 0; idx < CFG_RX_DESCR_NUM; idx++) {
		desc_p = &desc_table_p[idx];
		desc_p->sl = RX_OWEN_DMA;
		desc_p->buffer = (u32)&rxbuffs[idx * PKTSIZE_ALIGN];
		if (idx < (CFG_RX_DESCR_NUM - 1))
			desc_p->next = (u32)&desc_table_p[idx + 1];
		else
			desc_p->next = (u32)&priv->rdesc[0];
	}
	flush_dcache_range((ulong)&desc_table_p[0],
			   (ulong)&desc_table_p[CFG_RX_DESCR_NUM]);
}

static void npcm750_set_fifo_threshold(struct npcm750_eth_dev *priv)
{
	struct emc_regs *reg = priv->emc_regs_p;
	unsigned int val;

	val = RXTHD | TXTHD | BLENGTH;
	writel(val, &reg->fftcr);
}

static void npcm750_set_global_maccmd(struct npcm750_eth_dev *priv)
{
	struct emc_regs *reg = priv->emc_regs_p;
	unsigned int val;

	val = readl(&reg->mcmdr);
	val |= MCMDR_SPCRC | MCMDR_ENMDC | MCMDR_ACP | MCMDR_NDEF;
	writel(val, &reg->mcmdr);
}

static void npcm750_set_cam(struct npcm750_eth_dev *priv,
			    unsigned int x, unsigned char *pval)
{
	struct emc_regs *reg = priv->emc_regs_p;
	unsigned int msw, lsw;

	msw = (pval[0] << 24) | (pval[1] << 16) | (pval[2] << 8) | pval[3];
	lsw = (pval[4] << 24) | (pval[5] << 16);

	writel(lsw, &reg->cam0l + x * CAM_ENTRY_SIZE);
	writel(msw, &reg->cam0m + x * CAM_ENTRY_SIZE);
	writel(readl(&reg->camen) | CAM0EN, &reg->camen);
	writel(CAMCMR_ECMP | CAMCMR_ABP | CAMCMR_AUP, &reg->camcmr);
}

static void npcm750_adjust_link(struct emc_regs *reg,
				struct phy_device *phydev)
{
	u32 val = readl(&reg->mcmdr);

	if (!phydev->link) {
		printf("%s: No link.\n", phydev->dev->name);
		return;
	}

	if (phydev->speed == 100)
		val |= MCMDR_OPMOD;
	else
		val &= ~MCMDR_OPMOD;

	if (phydev->duplex)
		val |= MCMDR_FDUP;
	else
		val &= ~MCMDR_FDUP;

	writel(val, &reg->mcmdr);

	debug("Speed: %d, %s duplex%s\n", phydev->speed,
	      (phydev->duplex) ? "full" : "half",
	      (phydev->port == PORT_FIBRE) ? ", fiber mode" : "");
}

static int npcm750_phy_init(struct npcm750_eth_dev *priv, void *dev)
{
	struct phy_device *phydev;
	int ret;
	u32 address = 0x0;

	phydev = phy_connect(priv->bus, address, dev, priv->interface);
	if (!phydev)
		return -ENODEV;

	if (priv->max_speed) {
		ret = phy_set_supported(phydev, priv->max_speed);
		if (ret)
			return ret;
	}
	phydev->advertising = phydev->supported;

	priv->phydev = phydev;
	phy_config(phydev);
	return 0;
}

static int npcm750_eth_start(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct npcm750_eth_dev *priv = dev_get_priv(dev);
	struct emc_regs *reg = priv->emc_regs_p;
	u8 *enetaddr = pdata->enetaddr;
	int ret;

	writel(readl(&reg->mcmdr) & ~MCMDR_TXON  & ~MCMDR_RXON, &reg->mcmdr);

	writel(readl(&reg->mcmdr) | MCMDR_SWR, &reg->mcmdr);
	do {
		ret = readl(&reg->mcmdr);
	} while (ret & MCMDR_SWR);

	npcm750_rx_descs_init(priv);
	npcm750_tx_descs_init(priv);

	npcm750_set_cam(priv, priv->idx, enetaddr);
	npcm750_set_global_maccmd(priv);
	npcm750_set_fifo_threshold(priv);

	/* Start up the PHY */
	ret = phy_startup(priv->phydev);
	if (ret) {
		printf("Could not initialize PHY\n");
		return ret;
	}

	npcm750_adjust_link(reg, priv->phydev);
	writel(readl(&reg->mcmdr) | MCMDR_TXON | MCMDR_RXON, &reg->mcmdr);

	return 0;
}

static int npcm750_eth_send(struct udevice *dev, void *packet, int length)
{
	struct npcm750_eth_dev *priv = dev_get_priv(dev);
	struct emc_regs *reg = priv->emc_regs_p;
	struct npcm750_txbd *desc_p;
	struct npcm750_txbd *next_desc_p;

	desc_p = priv->curr_txd;

	invalidate_dcache_range((ulong)desc_p, (ulong)(desc_p + 1));
	/* Check if the descriptor is owned by CPU */
	if (desc_p->mode & TX_OWEN_DMA) {
		next_desc_p = (struct npcm750_txbd *)desc_p->next;

		while ((next_desc_p != desc_p) && (next_desc_p->mode & TX_OWEN_DMA))
			next_desc_p = (struct npcm750_txbd *)next_desc_p->next;

		if (next_desc_p == desc_p) {
			struct emc_regs *reg = priv->emc_regs_p;

			writel(0, &reg->tsdr);
			serial_printf("TX: overflow and exit\n");
			return -EPERM;
		}

		desc_p = next_desc_p;
	}

	memcpy((void *)desc_p->buffer, packet, length);
	flush_dcache_range((ulong)desc_p->buffer,
			   (ulong)desc_p->buffer + roundup(length, ARCH_DMA_MINALIGN));
	desc_p->sl = 0;
	desc_p->sl = length & TX_STAT_TBC;
	desc_p->mode = TX_OWEN_DMA | PADDINGMODE | CRCMODE;
	flush_dcache_range((ulong)desc_p, (ulong)(desc_p + 1));

	if (!(readl(&reg->mcmdr) & MCMDR_TXON))
		writel(readl(&reg->mcmdr) | MCMDR_TXON, &reg->mcmdr);
	priv->curr_txd = (struct npcm750_txbd *)priv->curr_txd->next;

	writel(0, &reg->tsdr);
	return 0;
}

static int npcm750_eth_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct npcm750_eth_dev *priv = dev_get_priv(dev);
	struct npcm750_rxbd *desc_p;
	struct npcm750_rxbd *next_desc_p;
	int length = -1;

	desc_p = priv->curr_rxd;
	invalidate_dcache_range((ulong)desc_p, (ulong)(desc_p + 1));

	if ((desc_p->sl & RX_STAT_OWNER) == RX_OWEN_DMA) {
		next_desc_p = (struct npcm750_rxbd *)desc_p->next;
		while ((next_desc_p != desc_p) &&
		       ((next_desc_p->sl & RX_STAT_OWNER) == RX_OWEN_CPU)) {
			next_desc_p = (struct npcm750_rxbd *)next_desc_p->next;
		}

		if (next_desc_p == desc_p) {
			struct emc_regs *reg = priv->emc_regs_p;

			writel(0, &reg->rsdr);
			serial_printf("RX: overflow and exit\n");
			return -EPERM;
		}
		desc_p = next_desc_p;
	}

	/* Check if the descriptor is owned by CPU */
	if ((desc_p->sl & RX_STAT_OWNER) == RX_OWEN_CPU) {
		if (desc_p->sl & RX_STAT_RXGD) {
			length = desc_p->sl & RX_STAT_RBC;
			invalidate_dcache_range((ulong)desc_p->buffer,
						(ulong)(desc_p->buffer + roundup(length,
						ARCH_DMA_MINALIGN)));
			*packetp = (u8 *)(u32)desc_p->buffer;
			priv->curr_rxd = desc_p;
		}
	}
	return length;
}

static int npcm750_eth_free_pkt(struct udevice *dev, uchar *packet, int length)
{
	struct npcm750_eth_dev *priv = dev_get_priv(dev);
	struct emc_regs *reg = priv->emc_regs_p;
	struct npcm750_rxbd *desc_p = priv->curr_rxd;

	/*
	 * Make the current descriptor valid again and go to
	 * the next one
	 */
	desc_p->sl |= RX_OWEN_DMA;
	flush_dcache_range((ulong)desc_p, (ulong)(desc_p + 1));
	priv->curr_rxd = (struct npcm750_rxbd *)priv->curr_rxd->next;
	writel(0, &reg->rsdr);

	return 0;
}

static void npcm750_eth_stop(struct udevice *dev)
{
	struct npcm750_eth_dev *priv = dev_get_priv(dev);
	struct emc_regs *reg = priv->emc_regs_p;

	writel(readl(&reg->mcmdr) & ~MCMDR_TXON, &reg->mcmdr);
	writel(readl(&reg->mcmdr) & ~MCMDR_RXON, &reg->mcmdr);
	priv->curr_txd = (struct npcm750_txbd *)readl(&reg->txdlsa);
	priv->curr_rxd = (struct npcm750_rxbd *)readl(&reg->rxdlsa);
	phy_shutdown(priv->phydev);
}

static int npcm750_eth_write_hwaddr(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct npcm750_eth_dev *priv = dev_get_priv(dev);

	npcm750_set_cam(priv, CAM0, pdata->enetaddr);
	return 0;
}

static int npcm750_eth_bind(struct udevice *dev)
{
	return 0;
}

static int npcm750_eth_probe(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct npcm750_eth_dev *priv = dev_get_priv(dev);
	u32 iobase = pdata->iobase;
	int ret;

	memset(priv, 0, sizeof(struct npcm750_eth_dev));
	ret = dev_read_u32(dev, "id", &priv->idx);
	if (ret) {
		printf("failed to get id\n");
		return -EINVAL;
	}

	priv->gcr_regmap = syscon_regmap_lookup_by_phandle(dev, "syscon-gcr");
	if (IS_ERR(priv->gcr_regmap))
		return -EINVAL;

	priv->emc_regs_p = (struct emc_regs *)iobase;
	priv->interface = pdata->phy_interface;
	priv->max_speed = pdata->max_speed;

	if (priv->idx == 0) {
		/* Enable RMII for EMC1 module */
		regmap_update_bits(priv->gcr_regmap, GCR_INTCR, INTCR_R1EN, INTCR_R1EN);
	}

	npcm750_mdio_init(dev->name, priv);
	priv->bus = miiphy_get_dev_by_name(dev->name);

	ret = npcm750_phy_init(priv, dev);

	return ret;
}

static int npcm750_eth_remove(struct udevice *dev)
{
	struct npcm750_eth_dev *priv = dev_get_priv(dev);

	free(priv->phydev);
	mdio_unregister(priv->bus);
	mdio_free(priv->bus);

	return 0;
}

static const struct eth_ops npcm750_eth_ops = {
	.start			= npcm750_eth_start,
	.send			= npcm750_eth_send,
	.recv			= npcm750_eth_recv,
	.free_pkt		= npcm750_eth_free_pkt,
	.stop			= npcm750_eth_stop,
	.write_hwaddr	= npcm750_eth_write_hwaddr,
};

static int npcm750_eth_ofdata_to_platdata(struct udevice *dev)
{
	struct npcm750_eth_pdata *npcm750_pdata = dev_get_plat(dev);
	struct eth_pdata *pdata = &npcm750_pdata->eth_pdata;
	const char *phy_mode;
	const fdt32_t *cell;
	int ret = 0;

	pdata->iobase = (phys_addr_t)dev_read_addr_ptr(dev);

	pdata->phy_interface = -1;
	phy_mode = fdt_getprop(gd->fdt_blob, dev_of_offset(dev), "phy-mode", NULL);

	if (phy_mode)
		pdata->phy_interface = dev_read_phy_mode(dev);

	if (pdata->phy_interface == PHY_INTERFACE_MODE_NA)
		return -EINVAL;

	pdata->max_speed = 0;
	cell = fdt_getprop(gd->fdt_blob, dev_of_offset(dev), "max-speed", NULL);
	if (cell)
		pdata->max_speed = fdt32_to_cpu(*cell);

	return ret;
}

static const struct udevice_id npcm750_eth_ids[] = {
	{ .compatible = "nuvoton,npcm750-emc" },
	{ }
};

U_BOOT_DRIVER(eth_npcm750) = {
	.name	= "eth_npcm750",
	.id	= UCLASS_ETH,
	.of_match = npcm750_eth_ids,
	.of_to_plat = npcm750_eth_ofdata_to_platdata,
	.bind	= npcm750_eth_bind,
	.probe	= npcm750_eth_probe,
	.remove	= npcm750_eth_remove,
	.ops	= &npcm750_eth_ops,
	.priv_auto = sizeof(struct npcm750_eth_dev),
	.plat_auto = sizeof(struct npcm750_eth_pdata),
	.flags = DM_FLAG_ALLOC_PRIV_DMA,
};
