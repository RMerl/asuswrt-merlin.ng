// SPDX-License-Identifier: GPL-2.0+
/*
 * Faraday 10/100Mbps Ethernet Controller
 *
 * (C) Copyright 2013 Faraday Technology
 * Dante Su <dantesu@faraday-tech.com>
 */

#include <common.h>
#include <command.h>
#include <malloc.h>
#include <net.h>
#include <linux/errno.h>
#include <asm/io.h>
#include <asm/dma-mapping.h>

#if defined(CONFIG_MII) || defined(CONFIG_CMD_MII)
#include <miiphy.h>
#endif

#include "ftmac110.h"

#define CFG_RXDES_NUM   8
#define CFG_TXDES_NUM   2
#define CFG_XBUF_SIZE   1536

#define CFG_MDIORD_TIMEOUT  (CONFIG_SYS_HZ >> 1) /* 500 ms */
#define CFG_MDIOWR_TIMEOUT  (CONFIG_SYS_HZ >> 1) /* 500 ms */
#define CFG_LINKUP_TIMEOUT  (CONFIG_SYS_HZ << 2) /* 4 sec */

/*
 * FTMAC110 DMA design issue
 *
 * Its DMA engine has a weird restriction that its Rx DMA engine
 * accepts only 16-bits aligned address, 32-bits aligned is not
 * acceptable. However this restriction does not apply to Tx DMA.
 *
 * Conclusion:
 * (1) Tx DMA Buffer Address:
 *     1 bytes aligned: Invalid
 *     2 bytes aligned: O.K
 *     4 bytes aligned: O.K (-> u-boot ZeroCopy is possible)
 * (2) Rx DMA Buffer Address:
 *     1 bytes aligned: Invalid
 *     2 bytes aligned: O.K
 *     4 bytes aligned: Invalid
 */

struct ftmac110_chip {
	void __iomem *regs;
	uint32_t imr;
	uint32_t maccr;
	uint32_t lnkup;
	uint32_t phy_addr;

	struct ftmac110_desc *rxd;
	ulong                rxd_dma;
	uint32_t             rxd_idx;

	struct ftmac110_desc *txd;
	ulong                txd_dma;
	uint32_t             txd_idx;
};

static int ftmac110_reset(struct eth_device *dev);

static uint16_t mdio_read(struct eth_device *dev,
	uint8_t phyaddr, uint8_t phyreg)
{
	struct ftmac110_chip *chip = dev->priv;
	struct ftmac110_regs *regs = chip->regs;
	uint32_t tmp, ts;
	uint16_t ret = 0xffff;

	tmp = PHYCR_READ
		| (phyaddr << PHYCR_ADDR_SHIFT)
		| (phyreg  << PHYCR_REG_SHIFT);

	writel(tmp, &regs->phycr);

	for (ts = get_timer(0); get_timer(ts) < CFG_MDIORD_TIMEOUT; ) {
		tmp = readl(&regs->phycr);
		if (tmp & PHYCR_READ)
			continue;
		break;
	}

	if (tmp & PHYCR_READ)
		printf("ftmac110: mdio read timeout\n");
	else
		ret = (uint16_t)(tmp & 0xffff);

	return ret;
}

static void mdio_write(struct eth_device *dev,
	uint8_t phyaddr, uint8_t phyreg, uint16_t phydata)
{
	struct ftmac110_chip *chip = dev->priv;
	struct ftmac110_regs *regs = chip->regs;
	uint32_t tmp, ts;

	tmp = PHYCR_WRITE
		| (phyaddr << PHYCR_ADDR_SHIFT)
		| (phyreg  << PHYCR_REG_SHIFT);

	writel(phydata, &regs->phydr);
	writel(tmp, &regs->phycr);

	for (ts = get_timer(0); get_timer(ts) < CFG_MDIOWR_TIMEOUT; ) {
		if (readl(&regs->phycr) & PHYCR_WRITE)
			continue;
		break;
	}

	if (readl(&regs->phycr) & PHYCR_WRITE)
		printf("ftmac110: mdio write timeout\n");
}

static uint32_t ftmac110_phyqry(struct eth_device *dev)
{
	ulong ts;
	uint32_t maccr;
	uint16_t pa, tmp, bmsr, bmcr;
	struct ftmac110_chip *chip = dev->priv;

	/* Default = 100Mbps Full */
	maccr = MACCR_100M | MACCR_FD;

	/* 1. find the phy device  */
	for (pa = 0; pa < 32; ++pa) {
		tmp = mdio_read(dev, pa, MII_PHYSID1);
		if (tmp == 0xFFFF || tmp == 0x0000)
			continue;
		chip->phy_addr = pa;
		break;
	}
	if (pa >= 32) {
		puts("ftmac110: phy device not found!\n");
		goto exit;
	}

	/* 2. wait until link-up & auto-negotiation complete */
	chip->lnkup = 0;
	bmcr = mdio_read(dev, chip->phy_addr, MII_BMCR);
	ts = get_timer(0);
	do {
		bmsr = mdio_read(dev, chip->phy_addr, MII_BMSR);
		chip->lnkup = (bmsr & BMSR_LSTATUS) ? 1 : 0;
		if (!chip->lnkup)
			continue;
		if (!(bmcr & BMCR_ANENABLE) || (bmsr & BMSR_ANEGCOMPLETE))
			break;
	} while (get_timer(ts) < CFG_LINKUP_TIMEOUT);
	if (!chip->lnkup) {
		puts("ftmac110: link down\n");
		goto exit;
	}
	if (!(bmcr & BMCR_ANENABLE))
		puts("ftmac110: auto negotiation disabled\n");
	else if (!(bmsr & BMSR_ANEGCOMPLETE))
		puts("ftmac110: auto negotiation timeout\n");

	/* 3. derive MACCR */
	if ((bmcr & BMCR_ANENABLE) && (bmsr & BMSR_ANEGCOMPLETE)) {
		tmp  = mdio_read(dev, chip->phy_addr, MII_ADVERTISE);
		tmp &= mdio_read(dev, chip->phy_addr, MII_LPA);
		if (tmp & LPA_100FULL)      /* 100Mbps full-duplex */
			maccr = MACCR_100M | MACCR_FD;
		else if (tmp & LPA_100HALF) /* 100Mbps half-duplex */
			maccr = MACCR_100M;
		else if (tmp & LPA_10FULL)  /* 10Mbps full-duplex */
			maccr = MACCR_FD;
		else if (tmp & LPA_10HALF)  /* 10Mbps half-duplex */
			maccr = 0;
	} else {
		if (bmcr & BMCR_SPEED100)
			maccr = MACCR_100M;
		else
			maccr = 0;
		if (bmcr & BMCR_FULLDPLX)
			maccr |= MACCR_FD;
	}

exit:
	printf("ftmac110: %d Mbps, %s\n",
	       (maccr & MACCR_100M) ? 100 : 10,
	       (maccr & MACCR_FD) ? "Full" : "half");
	return maccr;
}

static int ftmac110_reset(struct eth_device *dev)
{
	uint8_t *a;
	uint32_t i, maccr;
	struct ftmac110_chip *chip = dev->priv;
	struct ftmac110_regs *regs = chip->regs;

	/* 1. MAC reset */
	writel(MACCR_RESET, &regs->maccr);
	for (i = get_timer(0); get_timer(i) < 1000; ) {
		if (readl(&regs->maccr) & MACCR_RESET)
			continue;
		break;
	}
	if (readl(&regs->maccr) & MACCR_RESET) {
		printf("ftmac110: reset failed\n");
		return -ENXIO;
	}

	/* 1-1. Init tx ring */
	for (i = 0; i < CFG_TXDES_NUM; ++i) {
		/* owned by SW */
		chip->txd[i].ctrl &= cpu_to_le64(FTMAC110_TXD_CLRMASK);
	}
	chip->txd_idx = 0;

	/* 1-2. Init rx ring */
	for (i = 0; i < CFG_RXDES_NUM; ++i) {
		/* owned by HW */
		chip->rxd[i].ctrl &= cpu_to_le64(FTMAC110_RXD_CLRMASK);
		chip->rxd[i].ctrl |= cpu_to_le64(FTMAC110_RXD_OWNER);
	}
	chip->rxd_idx = 0;

	/* 2. PHY status query */
	maccr = ftmac110_phyqry(dev);

	/* 3. Fix up the MACCR value */
	chip->maccr = maccr | MACCR_CRCAPD | MACCR_RXALL | MACCR_RXRUNT
		| MACCR_RXEN | MACCR_TXEN | MACCR_RXDMAEN | MACCR_TXDMAEN;

	/* 4. MAC address setup */
	a = dev->enetaddr;
	writel(a[1] | (a[0] << 8), &regs->mac[0]);
	writel(a[5] | (a[4] << 8) | (a[3] << 16)
		| (a[2] << 24), &regs->mac[1]);

	/* 5. MAC registers setup */
	writel(chip->rxd_dma, &regs->rxba);
	writel(chip->txd_dma, &regs->txba);
	/* interrupt at each tx/rx */
	writel(ITC_DEFAULT, &regs->itc);
	/* no tx pool, rx poll = 1 normal cycle */
	writel(APTC_DEFAULT, &regs->aptc);
	/* rx threshold = [6/8 fifo, 2/8 fifo] */
	writel(DBLAC_DEFAULT, &regs->dblac);
	/* disable & clear all interrupt status */
	chip->imr = 0;
	writel(ISR_ALL, &regs->isr);
	writel(chip->imr, &regs->imr);
	/* enable mac */
	writel(chip->maccr, &regs->maccr);

	return 0;
}

static int ftmac110_probe(struct eth_device *dev, bd_t *bis)
{
	debug("ftmac110: probe\n");

	if (ftmac110_reset(dev))
		return -1;

	return 0;
}

static void ftmac110_halt(struct eth_device *dev)
{
	struct ftmac110_chip *chip = dev->priv;
	struct ftmac110_regs *regs = chip->regs;

	writel(0, &regs->imr);
	writel(0, &regs->maccr);

	debug("ftmac110: halt\n");
}

static int ftmac110_send(struct eth_device *dev, void *pkt, int len)
{
	struct ftmac110_chip *chip = dev->priv;
	struct ftmac110_regs *regs = chip->regs;
	struct ftmac110_desc *txd;
	uint64_t ctrl;

	if (!chip->lnkup)
		return 0;

	if (len <= 0 || len > CFG_XBUF_SIZE) {
		printf("ftmac110: bad tx pkt len(%d)\n", len);
		return 0;
	}

	len = max(60, len);

	txd = &chip->txd[chip->txd_idx];
	ctrl = le64_to_cpu(txd->ctrl);
	if (ctrl & FTMAC110_TXD_OWNER) {
		/* kick-off Tx DMA */
		writel(0xffffffff, &regs->txpd);
		printf("ftmac110: out of txd\n");
		return 0;
	}

	memcpy(txd->vbuf, (void *)pkt, len);
	dma_map_single(txd->vbuf, len, DMA_TO_DEVICE);

	/* clear control bits */
	ctrl &= FTMAC110_TXD_CLRMASK;
	/* set len, fts and lts */
	ctrl |= FTMAC110_TXD_LEN(len) | FTMAC110_TXD_FTS | FTMAC110_TXD_LTS;
	/* set owner bit */
	ctrl |= FTMAC110_TXD_OWNER;
	/* write back to descriptor */
	txd->ctrl = cpu_to_le64(ctrl);

	/* kick-off Tx DMA */
	writel(0xffffffff, &regs->txpd);

	chip->txd_idx = (chip->txd_idx + 1) % CFG_TXDES_NUM;

	return len;
}

static int ftmac110_recv(struct eth_device *dev)
{
	struct ftmac110_chip *chip = dev->priv;
	struct ftmac110_desc *rxd;
	uint32_t len, rlen = 0;
	uint64_t ctrl;
	uint8_t *buf;

	if (!chip->lnkup)
		return 0;

	do {
		rxd = &chip->rxd[chip->rxd_idx];
		ctrl = le64_to_cpu(rxd->ctrl);
		if (ctrl & FTMAC110_RXD_OWNER)
			break;

		len = (uint32_t)FTMAC110_RXD_LEN(ctrl);
		buf = rxd->vbuf;

		if (ctrl & FTMAC110_RXD_ERRMASK) {
			printf("ftmac110: rx error\n");
		} else {
			dma_map_single(buf, len, DMA_FROM_DEVICE);
			net_process_received_packet(buf, len);
			rlen += len;
		}

		/* owned by hardware */
		ctrl &= FTMAC110_RXD_CLRMASK;
		ctrl |= FTMAC110_RXD_OWNER;
		rxd->ctrl |= cpu_to_le64(ctrl);

		chip->rxd_idx = (chip->rxd_idx + 1) % CFG_RXDES_NUM;
	} while (0);

	return rlen;
}

#if defined(CONFIG_MII) || defined(CONFIG_CMD_MII)

static int ftmac110_mdio_read(struct mii_dev *bus, int addr, int devad,
			      int reg)
{
	uint16_t value = 0;
	int ret = 0;
	struct eth_device *dev;

	dev = eth_get_dev_by_name(bus->name);
	if (dev == NULL) {
		printf("%s: no such device\n", bus->name);
		ret = -1;
	} else {
		value = mdio_read(dev, addr, reg);
	}

	if (ret < 0)
		return ret;
	return value;
}

static int ftmac110_mdio_write(struct mii_dev *bus, int addr, int devad,
			       int reg, u16 value)
{
	int ret = 0;
	struct eth_device *dev;

	dev = eth_get_dev_by_name(bus->name);
	if (dev == NULL) {
		printf("%s: no such device\n", bus->name);
		ret = -1;
	} else {
		mdio_write(dev, addr, reg, value);
	}

	return ret;
}

#endif    /* #if defined(CONFIG_MII) || defined(CONFIG_CMD_MII) */

int ftmac110_initialize(bd_t *bis)
{
	int i, card_nr = 0;
	struct eth_device *dev;
	struct ftmac110_chip *chip;

	dev = malloc(sizeof(*dev) + sizeof(*chip));
	if (dev == NULL) {
		panic("ftmac110: out of memory 1\n");
		return -1;
	}
	chip = (struct ftmac110_chip *)(dev + 1);
	memset(dev, 0, sizeof(*dev) + sizeof(*chip));

	sprintf(dev->name, "FTMAC110#%d", card_nr);

	dev->iobase = CONFIG_FTMAC110_BASE;
	chip->regs = (void __iomem *)dev->iobase;
	dev->priv = chip;
	dev->init = ftmac110_probe;
	dev->halt = ftmac110_halt;
	dev->send = ftmac110_send;
	dev->recv = ftmac110_recv;

	/* allocate tx descriptors (it must be 16 bytes aligned) */
	chip->txd = dma_alloc_coherent(
		sizeof(struct ftmac110_desc) * CFG_TXDES_NUM, &chip->txd_dma);
	if (!chip->txd)
		panic("ftmac110: out of memory 3\n");
	memset(chip->txd, 0,
	       sizeof(struct ftmac110_desc) * CFG_TXDES_NUM);
	for (i = 0; i < CFG_TXDES_NUM; ++i) {
		void *va = memalign(ARCH_DMA_MINALIGN, CFG_XBUF_SIZE);

		if (!va)
			panic("ftmac110: out of memory 4\n");
		chip->txd[i].vbuf = va;
		chip->txd[i].pbuf = cpu_to_le32(virt_to_phys(va));
		chip->txd[i].ctrl = 0;	/* owned by SW */
	}
	chip->txd[i - 1].ctrl |= cpu_to_le64(FTMAC110_TXD_END);
	chip->txd_idx = 0;

	/* allocate rx descriptors (it must be 16 bytes aligned) */
	chip->rxd = dma_alloc_coherent(
		sizeof(struct ftmac110_desc) * CFG_RXDES_NUM, &chip->rxd_dma);
	if (!chip->rxd)
		panic("ftmac110: out of memory 4\n");
	memset((void *)chip->rxd, 0,
	       sizeof(struct ftmac110_desc) * CFG_RXDES_NUM);
	for (i = 0; i < CFG_RXDES_NUM; ++i) {
		void *va = memalign(ARCH_DMA_MINALIGN, CFG_XBUF_SIZE + 2);

		if (!va)
			panic("ftmac110: out of memory 5\n");
		/* it needs to be exactly 2 bytes aligned */
		va = ((uint8_t *)va + 2);
		chip->rxd[i].vbuf = va;
		chip->rxd[i].pbuf = cpu_to_le32(virt_to_phys(va));
		chip->rxd[i].ctrl = cpu_to_le64(FTMAC110_RXD_OWNER
			| FTMAC110_RXD_BUFSZ(CFG_XBUF_SIZE));
	}
	chip->rxd[i - 1].ctrl |= cpu_to_le64(FTMAC110_RXD_END);
	chip->rxd_idx = 0;

	eth_register(dev);

#if defined(CONFIG_MII) || defined(CONFIG_CMD_MII)
	int retval;
	struct mii_dev *mdiodev = mdio_alloc();
	if (!mdiodev)
		return -ENOMEM;
	strncpy(mdiodev->name, dev->name, MDIO_NAME_LEN);
	mdiodev->read = ftmac110_mdio_read;
	mdiodev->write = ftmac110_mdio_write;

	retval = mdio_register(mdiodev);
	if (retval < 0)
		return retval;
#endif

	card_nr++;

	return card_nr;
}
