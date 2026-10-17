// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2014-2017 Broadcom.
 */

#ifdef BCM_GMAC_DEBUG
#ifndef DEBUG
#define DEBUG
#include <linux/printk.h>
#endif
#endif

#include <config.h>
#include <common.h>
#include <cpu_func.h>
#include <log.h>
#include <malloc.h>
#include <net.h>
#include <asm/cache.h>
#include <asm/io.h>
#include <phy.h>
#include <linux/delay.h>
#include <linux/bitops.h>

#include "bcm-sf2-eth.h"
#include "bcm-sf2-eth-gmac.h"

#define SPINWAIT(exp, us) { \
	uint countdown = (us) + 9; \
	while ((exp) && (countdown >= 10)) {\
		udelay(10); \
		countdown -= 10; \
	} \
}

#define RX_BUF_SIZE_ALIGNED	ALIGN(RX_BUF_SIZE, ARCH_DMA_MINALIGN)
#define TX_BUF_SIZE_ALIGNED	ALIGN(TX_BUF_SIZE, ARCH_DMA_MINALIGN)
#define DESCP_SIZE_ALIGNED	ALIGN(sizeof(dma64dd_t), ARCH_DMA_MINALIGN)

static int gmac_disable_dma(struct eth_dma *dma, int dir);
static int gmac_enable_dma(struct eth_dma *dma, int dir);

/* DMA Descriptor */
typedef struct {
	/* misc control bits */
	uint32_t	ctrl1;
	/* buffer count and address extension */
	uint32_t	ctrl2;
	/* memory address of the date buffer, bits 31:0 */
	uint32_t	addrlow;
	/* memory address of the date buffer, bits 63:32 */
	uint32_t	addrhigh;
} dma64dd_t;

uint32_t g_dmactrlflags;

static uint32_t dma_ctrlflags(uint32_t mask, uint32_t flags)
{
	debug("%s enter\n", __func__);

	g_dmactrlflags &= ~mask;
	g_dmactrlflags |= flags;

	/* If trying to enable parity, check if parity is actually supported */
	if (g_dmactrlflags & DMA_CTRL_PEN) {
		uint32_t control;

		control = readl(GMAC0_DMA_TX_CTRL_ADDR);
		writel(control | D64_XC_PD, GMAC0_DMA_TX_CTRL_ADDR);
		if (readl(GMAC0_DMA_TX_CTRL_ADDR) & D64_XC_PD) {
			/*
			 * We *can* disable it, therefore it is supported;
			 * restore control register
			 */
			writel(control, GMAC0_DMA_TX_CTRL_ADDR);
		} else {
			/* Not supported, don't allow it to be enabled */
			g_dmactrlflags &= ~DMA_CTRL_PEN;
		}
	}

	return g_dmactrlflags;
}

static inline void reg32_clear_bits(uint32_t reg, uint32_t value)
{
	uint32_t v = readl(reg);
	v &= ~(value);
	writel(v, reg);
}

static inline void reg32_set_bits(uint32_t reg, uint32_t value)
{
	uint32_t v = readl(reg);
	v |= value;
	writel(v, reg);
}

#ifdef BCM_GMAC_DEBUG
static void dma_tx_dump(struct eth_dma *dma)
{
	dma64dd_t *descp = NULL;
	uint8_t *bufp;
	int i;

	printf("TX DMA Register:\n");
	printf("control:0x%x; ptr:0x%x; addrl:0x%x; addrh:0x%x; stat0:0x%x, stat1:0x%x\n",
	       readl(GMAC0_DMA_TX_CTRL_ADDR),
	       readl(GMAC0_DMA_TX_PTR_ADDR),
	       readl(GMAC0_DMA_TX_ADDR_LOW_ADDR),
	       readl(GMAC0_DMA_TX_ADDR_HIGH_ADDR),
	       readl(GMAC0_DMA_TX_STATUS0_ADDR),
	       readl(GMAC0_DMA_TX_STATUS1_ADDR));

	printf("TX Descriptors:\n");
	for (i = 0; i < TX_BUF_NUM; i++) {
		descp = (dma64dd_t *)(dma->tx_desc_aligned) + i;
		printf("ctrl1:0x%08x; ctrl2:0x%08x; addr:0x%x 0x%08x\n",
		       descp->ctrl1, descp->ctrl2,
		       descp->addrhigh, descp->addrlow);
	}

	printf("TX Buffers:\n");
	/* Initialize TX DMA descriptor table */
	for (i = 0; i < TX_BUF_NUM; i++) {
		bufp = (uint8_t *)(dma->tx_buf + i * TX_BUF_SIZE_ALIGNED);
		printf("buf%d:0x%x; ", i, (uint32_t)bufp);
	}
	printf("\n");
}

static void dma_rx_dump(struct eth_dma *dma)
{
	dma64dd_t *descp = NULL;
	uint8_t *bufp;
	int i;

	printf("RX DMA Register:\n");
	printf("control:0x%x; ptr:0x%x; addrl:0x%x; addrh:0x%x; stat0:0x%x, stat1:0x%x\n",
	       readl(GMAC0_DMA_RX_CTRL_ADDR),
	       readl(GMAC0_DMA_RX_PTR_ADDR),
	       readl(GMAC0_DMA_RX_ADDR_LOW_ADDR),
	       readl(GMAC0_DMA_RX_ADDR_HIGH_ADDR),
	       readl(GMAC0_DMA_RX_STATUS0_ADDR),
	       readl(GMAC0_DMA_RX_STATUS1_ADDR));

	printf("RX Descriptors:\n");
	for (i = 0; i < RX_BUF_NUM; i++) {
		descp = (dma64dd_t *)(dma->rx_desc_aligned) + i;
		printf("ctrl1:0x%08x; ctrl2:0x%08x; addr:0x%x 0x%08x\n",
		       descp->ctrl1, descp->ctrl2,
		       descp->addrhigh, descp->addrlow);
	}

	printf("RX Buffers:\n");
	for (i = 0; i < RX_BUF_NUM; i++) {
		bufp = dma->rx_buf + i * RX_BUF_SIZE_ALIGNED;
		printf("buf%d:0x%x; ", i, (uint32_t)bufp);
	}
	printf("\n");
}
#endif

static int dma_tx_init(struct eth_dma *dma)
{
	dma64dd_t *descp = NULL;
	uint8_t *bufp;
	int i;
	uint32_t ctrl;

	debug("%s enter\n", __func__);

	/* clear descriptor memory */
	memset((void *)(dma->tx_desc_aligned), 0,
	       TX_BUF_NUM * DESCP_SIZE_ALIGNED);
	memset(dma->tx_buf, 0, TX_BUF_NUM * TX_BUF_SIZE_ALIGNED);

	/* Initialize TX DMA descriptor table */
	for (i = 0; i < TX_BUF_NUM; i++) {
		descp = (dma64dd_t *)(dma->tx_desc_aligned) + i;
		bufp = dma->tx_buf + i * TX_BUF_SIZE_ALIGNED;
		/* clear buffer memory */
		memset((void *)bufp, 0, TX_BUF_SIZE_ALIGNED);

		ctrl = 0;
		/* if last descr set endOfTable */
		if (i == (TX_BUF_NUM-1))
			ctrl = D64_CTRL1_EOT;
		descp->ctrl1 = ctrl;
		descp->ctrl2 = 0;
		descp->addrlow = (uint32_t)bufp;
		descp->addrhigh = 0;
	}

	/* flush descriptor and buffer */
	descp = dma->tx_desc_aligned;
	bufp = dma->tx_buf;
	flush_dcache_range((unsigned long)descp,
			   (unsigned long)descp +
			   DESCP_SIZE_ALIGNED * TX_BUF_NUM);
	flush_dcache_range((unsigned long)bufp,
			   (unsigned long)bufp +
			   TX_BUF_SIZE_ALIGNED * TX_BUF_NUM);

	/* initialize the DMA channel */
	writel((uint32_t)(dma->tx_desc_aligned), GMAC0_DMA_TX_ADDR_LOW_ADDR);
	writel(0, GMAC0_DMA_TX_ADDR_HIGH_ADDR);

	/* now update the dma last descriptor */
	writel(((uint32_t)(dma->tx_desc_aligned)) & D64_XP_LD_MASK,
	       GMAC0_DMA_TX_PTR_ADDR);

	return 0;
}

static int dma_rx_init(struct eth_dma *dma)
{
	uint32_t last_desc;
	dma64dd_t *descp = NULL;
	uint8_t *bufp;
	uint32_t ctrl;
	int i;

	debug("%s enter\n", __func__);

	/* clear descriptor memory */
	memset((void *)(dma->rx_desc_aligned), 0,
	       RX_BUF_NUM * DESCP_SIZE_ALIGNED);
	/* clear buffer memory */
	memset(dma->rx_buf, 0, RX_BUF_NUM * RX_BUF_SIZE_ALIGNED);

	/* Initialize RX DMA descriptor table */
	for (i = 0; i < RX_BUF_NUM; i++) {
		descp = (dma64dd_t *)(dma->rx_desc_aligned) + i;
		bufp = dma->rx_buf + i * RX_BUF_SIZE_ALIGNED;
		ctrl = 0;
		/* if last descr set endOfTable */
		if (i == (RX_BUF_NUM - 1))
			ctrl = D64_CTRL1_EOT;
		descp->ctrl1 = ctrl;
		descp->ctrl2 = RX_BUF_SIZE_ALIGNED;
		descp->addrlow = (uint32_t)bufp;
		descp->addrhigh = 0;

		last_desc = ((uint32_t)(descp) & D64_XP_LD_MASK)
				+ sizeof(dma64dd_t);
	}

	descp = dma->rx_desc_aligned;
	bufp = dma->rx_buf;
	/* flush descriptor and buffer */
	flush_dcache_range((unsigned long)descp,
			   (unsigned long)descp +
			   DESCP_SIZE_ALIGNED * RX_BUF_NUM);
	flush_dcache_range((unsigned long)(bufp),
			   (unsigned long)bufp +
			   RX_BUF_SIZE_ALIGNED * RX_BUF_NUM);

	/* initailize the DMA channel */
	writel((uint32_t)descp, GMAC0_DMA_RX_ADDR_LOW_ADDR);
	writel(0, GMAC0_DMA_RX_ADDR_HIGH_ADDR);

	/* now update the dma last descriptor */
	writel(last_desc, GMAC0_DMA_RX_PTR_ADDR);

	return 0;
}

static int dma_init(struct eth_dma *dma)
{
	debug(" %s enter\n", __func__);

	/*
	 * Default flags: For backwards compatibility both
	 * Rx Overflow Continue and Parity are DISABLED.
	 */
	dma_ctrlflags(DMA_CTRL_ROC | DMA_CTRL_PEN, 0);

	debug("rx burst len 0x%x\n",
	      (readl(GMAC0_DMA_RX_CTRL_ADDR) & D64_RC_BL_MASK)
	      >> D64_RC_BL_SHIFT);
	debug("tx burst len 0x%x\n",
	      (readl(GMAC0_DMA_TX_CTRL_ADDR) & D64_XC_BL_MASK)
	      >> D64_XC_BL_SHIFT);

	dma_tx_init(dma);
	dma_rx_init(dma);

	/* From end of chip_init() */
	/* enable the overflow continue feature and disable parity */
	dma_ctrlflags(DMA_CTRL_ROC | DMA_CTRL_PEN /* mask */,
		      DMA_CTRL_ROC /* value */);

	return 0;
}

static int dma_deinit(struct eth_dma *dma)
{
	debug(" %s enter\n", __func__);

	gmac_disable_dma(dma, MAC_DMA_RX);
	gmac_disable_dma(dma, MAC_DMA_TX);

	free(dma->tx_buf);
	dma->tx_buf = NULL;
	free(dma->tx_desc_aligned);
	dma->tx_desc_aligned = NULL;

	free(dma->rx_buf);
	dma->rx_buf = NULL;
	free(dma->rx_desc_aligned);
	dma->rx_desc_aligned = NULL;

	return 0;
}

int gmac_tx_packet(struct eth_dma *dma, void *packet, int length)
{
	uint8_t *bufp = dma->tx_buf + dma->cur_tx_index * TX_BUF_SIZE_ALIGNED;

	/* kick off the dma */
	size_t len = length;
	int txout = dma->cur_tx_index;
	uint32_t flags;
	dma64dd_t *descp = NULL;
	uint32_t ctrl;
	uint32_t last_desc = (((uint32_t)dma->tx_desc_aligned) +
			      sizeof(dma64dd_t)) & D64_XP_LD_MASK;
	size_t buflen;

	debug("%s enter\n", __func__);

	/* load the buffer */
	memcpy(bufp, packet, len);

	/* Add 4 bytes for Ethernet FCS/CRC */
	buflen = len + 4;

	ctrl = (buflen & D64_CTRL2_BC_MASK);

	/* the transmit will only be one frame or set SOF, EOF */
	/* also set int on completion */
	flags = D64_CTRL1_SOF | D64_CTRL1_IOC | D64_CTRL1_EOF;

	/* txout points to the descriptor to uset */
	/* if last descriptor then set EOT */
	if (txout == (TX_BUF_NUM - 1)) {
		flags |= D64_CTRL1_EOT;
		last_desc = ((uint32_t)(dma->tx_desc_aligned)) & D64_XP_LD_MASK;
	}

	/* write the descriptor */
	descp = ((dma64dd_t *)(dma->tx_desc_aligned)) + txout;
	descp->addrlow = (uint32_t)bufp;
	descp->addrhigh = 0;
	descp->ctrl1 = flags;
	descp->ctrl2 = ctrl;

	/* flush descriptor and buffer */
	flush_dcache_range((unsigned long)dma->tx_desc_aligned,
			   (unsigned long)dma->tx_desc_aligned +
			   DESCP_SIZE_ALIGNED * TX_BUF_NUM);
	flush_dcache_range((unsigned long)bufp,
			   (unsigned long)bufp + TX_BUF_SIZE_ALIGNED);

	/* now update the dma last descriptor */
	writel(last_desc, GMAC0_DMA_TX_PTR_ADDR);

	/* tx dma should be enabled so packet should go out */

	/* update txout */
	dma->cur_tx_index = (txout + 1) & (TX_BUF_NUM - 1);

	return 0;
}

bool gmac_check_tx_done(struct eth_dma *dma)
{
	/* wait for tx to complete */
	uint32_t intstatus;
	bool xfrdone = false;

	debug("%s enter\n", __func__);

	intstatus = readl(GMAC0_INT_STATUS_ADDR);

	debug("int(0x%x)\n", intstatus);
	if (intstatus & (I_XI0 | I_XI1 | I_XI2 | I_XI3)) {
		xfrdone = true;
		/* clear the int bits */
		intstatus &= ~(I_XI0 | I_XI1 | I_XI2 | I_XI3);
		writel(intstatus, GMAC0_INT_STATUS_ADDR);
	} else {
		debug("Tx int(0x%x)\n", intstatus);
	}

	return xfrdone;
}

int gmac_check_rx_done(struct eth_dma *dma, uint8_t *buf)
{
	void *bufp, *datap;
	size_t rcvlen = 0, buflen = 0;
	uint32_t stat0 = 0, stat1 = 0;
	uint32_t control, offset;
	uint8_t statbuf[HWRXOFF*2];

	int index, curr, active;
	dma64dd_t *descp = NULL;

	/* udelay(50); */

	/*
	 * this api will check if a packet has been received.
	 * If so it will return the address of the buffer and current
	 * descriptor index will be incremented to the
	 * next descriptor. Once done with the frame the buffer should be
	 * added back onto the descriptor and the lastdscr should be updated
	 * to this descriptor.
	 */
	index = dma->cur_rx_index;
	offset = (uint32_t)(dma->rx_desc_aligned);
	stat0 = readl(GMAC0_DMA_RX_STATUS0_ADDR) & D64_RS0_CD_MASK;
	stat1 = readl(GMAC0_DMA_RX_STATUS1_ADDR) & D64_RS0_CD_MASK;
	curr = ((stat0 - offset) & D64_RS0_CD_MASK) / sizeof(dma64dd_t);
	active = ((stat1 - offset) & D64_RS0_CD_MASK) / sizeof(dma64dd_t);

	/* check if any frame */
	if (index == curr)
		return -1;

	debug("received packet\n");
	debug("expect(0x%x) curr(0x%x) active(0x%x)\n", index, curr, active);
	/* remove warning */
	if (index == active)
		;

	/* get the packet pointer that corresponds to the rx descriptor */
	bufp = dma->rx_buf + index * RX_BUF_SIZE_ALIGNED;

	descp = (dma64dd_t *)(dma->rx_desc_aligned) + index;
	/* flush descriptor and buffer */
	flush_dcache_range((unsigned long)dma->rx_desc_aligned,
			   (unsigned long)dma->rx_desc_aligned +
			   DESCP_SIZE_ALIGNED * RX_BUF_NUM);
	flush_dcache_range((unsigned long)bufp,
			   (unsigned long)bufp + RX_BUF_SIZE_ALIGNED);

	buflen = (descp->ctrl2 & D64_CTRL2_BC_MASK);

	stat0 = readl(GMAC0_DMA_RX_STATUS0_ADDR);
	stat1 = readl(GMAC0_DMA_RX_STATUS1_ADDR);

	debug("bufp(0x%x) index(0x%x) buflen(0x%x) stat0(0x%x) stat1(0x%x)\n",
	      (uint32_t)bufp, index, buflen, stat0, stat1);

	dma->cur_rx_index = (index + 1) & (RX_BUF_NUM - 1);

	/* get buffer offset */
	control = readl(GMAC0_DMA_RX_CTRL_ADDR);
	offset = (control & D64_RC_RO_MASK) >> D64_RC_RO_SHIFT;
	rcvlen = *(uint16_t *)bufp;

	debug("Received %d bytes\n", rcvlen);
	/* copy status into temp buf then copy data from rx buffer */
	memcpy(statbuf, bufp, offset);
	datap = (void *)((uint32_t)bufp + offset);
	memcpy(buf, datap, rcvlen);

	/* update descriptor that is being added back on ring */
	descp->ctrl2 = RX_BUF_SIZE_ALIGNED;
	descp->addrlow = (uint32_t)bufp;
	descp->addrhigh = 0;
	/* flush descriptor */
	flush_dcache_range((unsigned long)dma->rx_desc_aligned,
			   (unsigned long)dma->rx_desc_aligned +
			   DESCP_SIZE_ALIGNED * RX_BUF_NUM);

	/* set the lastdscr for the rx ring */
	writel(((uint32_t)descp) & D64_XP_LD_MASK, GMAC0_DMA_RX_PTR_ADDR);

	return (int)rcvlen;
}

static int gmac_disable_dma(struct eth_dma *dma, int dir)
{
	int status;

	debug("%s enter\n", __func__);

	if (dir == MAC_DMA_TX) {
		/* address PR8249/PR7577 issue */
		/* suspend tx DMA first */
		writel(D64_XC_SE, GMAC0_DMA_TX_CTRL_ADDR);
		SPINWAIT(((status = (readl(GMAC0_DMA_TX_STATUS0_ADDR) &
				     D64_XS0_XS_MASK)) !=
			  D64_XS0_XS_DISABLED) &&
			 (status != D64_XS0_XS_IDLE) &&
			 (status != D64_XS0_XS_STOPPED), 10000);

		/*
		 * PR2414 WAR: DMA engines are not disabled until
		 * transfer finishes
		 */
		writel(0, GMAC0_DMA_TX_CTRL_ADDR);
		SPINWAIT(((status = (readl(GMAC0_DMA_TX_STATUS0_ADDR) &
				     D64_XS0_XS_MASK)) !=
			  D64_XS0_XS_DISABLED), 10000);

		/* wait for the last transaction to complete */
		udelay(2);

		status = (status == D64_XS0_XS_DISABLED);
	} else {
		/*
		 * PR2414 WAR: DMA engines are not disabled until
		 * transfer finishes
		 */
		writel(0, GMAC0_DMA_RX_CTRL_ADDR);
		SPINWAIT(((status = (readl(GMAC0_DMA_RX_STATUS0_ADDR) &
				     D64_RS0_RS_MASK)) !=
			  D64_RS0_RS_DISABLED), 10000);

		status = (status == D64_RS0_RS_DISABLED);
	}

	return status;
}

static int gmac_enable_dma(struct eth_dma *dma, int dir)
{
	uint32_t control;

	debug("%s enter\n", __func__);

	if (dir == MAC_DMA_TX) {
		dma->cur_tx_index = 0;

		/*
		 * These bits 20:18 (burstLen) of control register can be
		 * written but will take effect only if these bits are
		 * valid. So this will not affect previous versions
		 * of the DMA. They will continue to have those bits set to 0.
		 */
		control = readl(GMAC0_DMA_TX_CTRL_ADDR);

		control |= D64_XC_XE;
		if ((g_dmactrlflags & DMA_CTRL_PEN) == 0)
			control |= D64_XC_PD;

		writel(control, GMAC0_DMA_TX_CTRL_ADDR);

		/* initailize the DMA channel */
		writel((uint32_t)(dma->tx_desc_aligned),
		       GMAC0_DMA_TX_ADDR_LOW_ADDR);
		writel(0, GMAC0_DMA_TX_ADDR_HIGH_ADDR);
	} else {
		dma->cur_rx_index = 0;

		control = (readl(GMAC0_DMA_RX_CTRL_ADDR) &
			   D64_RC_AE) | D64_RC_RE;

		if ((g_dmactrlflags & DMA_CTRL_PEN) == 0)
			control |= D64_RC_PD;

		if (g_dmactrlflags & DMA_CTRL_ROC)
			control |= D64_RC_OC;

		/*
		 * These bits 20:18 (burstLen) of control register can be
		 * written but will take effect only if these bits are
		 * valid. So this will not affect previous versions
		 * of the DMA. They will continue to have those bits set to 0.
		 */
		control &= ~D64_RC_BL_MASK;
		/* Keep default Rx burstlen */
		control |= readl(GMAC0_DMA_RX_CTRL_ADDR) & D64_RC_BL_MASK;
		control |= HWRXOFF << D64_RC_RO_SHIFT;

		writel(control, GMAC0_DMA_RX_CTRL_ADDR);

		/*
		 * the rx descriptor ring should have
		 * the addresses set properly;
		 * set the lastdscr for the rx ring
		 */
		writel(((uint32_t)(dma->rx_desc_aligned) +
			(RX_BUF_NUM - 1) * RX_BUF_SIZE_ALIGNED) &
		       D64_XP_LD_MASK, GMAC0_DMA_RX_PTR_ADDR);
	}

	return 0;
}

bool gmac_mii_busywait(unsigned int timeout)
{
	uint32_t tmp = 0;

	while (timeout > 10) {
		tmp = readl(GMAC_MII_CTRL_ADDR);
		if (tmp & (1 << GMAC_MII_BUSY_SHIFT)) {
			udelay(10);
			timeout -= 10;
		} else {
			break;
		}
	}
	return tmp & (1 << GMAC_MII_BUSY_SHIFT);
}

int gmac_miiphy_read(struct mii_dev *bus, int phyaddr, int devad, int reg)
{
	uint32_t tmp = 0;
	u16 value = 0;

	/* Busy wait timeout is 1ms */
	if (gmac_mii_busywait(1000)) {
		pr_err("%s: Prepare MII read: MII/MDIO busy\n", __func__);
		return -1;
	}

	/* Read operation */
	tmp = GMAC_MII_DATA_READ_CMD;
	tmp |= (phyaddr << GMAC_MII_PHY_ADDR_SHIFT) |
		(reg << GMAC_MII_PHY_REG_SHIFT);
	debug("MII read cmd 0x%x, phy 0x%x, reg 0x%x\n", tmp, phyaddr, reg);
	writel(tmp, GMAC_MII_DATA_ADDR);

	if (gmac_mii_busywait(1000)) {
		pr_err("%s: MII read failure: MII/MDIO busy\n", __func__);
		return -1;
	}

	value = readl(GMAC_MII_DATA_ADDR) & 0xffff;
	debug("MII read data 0x%x\n", value);
	return value;
}

int gmac_miiphy_write(struct mii_dev *bus, int phyaddr, int devad, int reg,
		      u16 value)
{
	uint32_t tmp = 0;

	/* Busy wait timeout is 1ms */
	if (gmac_mii_busywait(1000)) {
		pr_err("%s: Prepare MII write: MII/MDIO busy\n", __func__);
		return -1;
	}

	/* Write operation */
	tmp = GMAC_MII_DATA_WRITE_CMD | (value & 0xffff);
	tmp |= ((phyaddr << GMAC_MII_PHY_ADDR_SHIFT) |
		(reg << GMAC_MII_PHY_REG_SHIFT));
	debug("MII write cmd 0x%x, phy 0x%x, reg 0x%x, data 0x%x\n",
	      tmp, phyaddr, reg, value);
	writel(tmp, GMAC_MII_DATA_ADDR);

	if (gmac_mii_busywait(1000)) {
		pr_err("%s: MII write failure: MII/MDIO busy\n", __func__);
		return -1;
	}

	return 0;
}

void gmac_init_reset(void)
{
	debug("%s enter\n", __func__);

	/* set command config reg CC_SR */
	reg32_set_bits(UNIMAC0_CMD_CFG_ADDR, CC_SR);
	udelay(GMAC_RESET_DELAY);
}

void gmac_clear_reset(void)
{
	debug("%s enter\n", __func__);

	/* clear command config reg CC_SR */
	reg32_clear_bits(UNIMAC0_CMD_CFG_ADDR, CC_SR);
	udelay(GMAC_RESET_DELAY);
}

static void gmac_enable_local(bool en)
{
	uint32_t cmdcfg;

	debug("%s enter\n", __func__);

	/* read command config reg */
	cmdcfg = readl(UNIMAC0_CMD_CFG_ADDR);

	/* put mac in reset */
	gmac_init_reset();

	cmdcfg |= CC_SR;

	/* first deassert rx_ena and tx_ena while in reset */
	cmdcfg &= ~(CC_RE | CC_TE);
	/* write command config reg */
	writel(cmdcfg, UNIMAC0_CMD_CFG_ADDR);

	/* bring mac out of reset */
	gmac_clear_reset();

	/* if not enable exit now */
	if (!en)
		return;

	/* enable the mac transmit and receive paths now */
	udelay(2);
	cmdcfg &= ~CC_SR;
	cmdcfg |= (CC_RE | CC_TE);

	/* assert rx_ena and tx_ena when out of reset to enable the mac */
	writel(cmdcfg, UNIMAC0_CMD_CFG_ADDR);

	return;
}

int gmac_enable(void)
{
	gmac_enable_local(1);

	/* clear interrupts */
	writel(I_INTMASK, GMAC0_INT_STATUS_ADDR);
	return 0;
}

int gmac_disable(void)
{
	gmac_enable_local(0);
	return 0;
}

int gmac_set_speed(int speed, int duplex)
{
	uint32_t cmdcfg;
	uint32_t hd_ena;
	uint32_t speed_cfg;

	hd_ena = duplex ? 0 : CC_HD;
	if (speed == 1000) {
		speed_cfg = 2;
	} else if (speed == 100) {
		speed_cfg = 1;
	} else if (speed == 10) {
		speed_cfg = 0;
	} else {
		pr_err("%s: Invalid GMAC speed(%d)!\n", __func__, speed);
		return -1;
	}

	cmdcfg = readl(UNIMAC0_CMD_CFG_ADDR);
	cmdcfg &= ~(CC_ES_MASK | CC_HD);
	cmdcfg |= ((speed_cfg << CC_ES_SHIFT) | hd_ena);

	printf("Change GMAC speed to %dMB\n", speed);
	debug("GMAC speed cfg 0x%x\n", cmdcfg);
	writel(cmdcfg, UNIMAC0_CMD_CFG_ADDR);

	return 0;
}

int gmac_set_mac_addr(unsigned char *mac)
{
	/* set our local address */
	debug("GMAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
	      mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	writel(htonl(*(uint32_t *)mac), UNIMAC0_MAC_MSB_ADDR);
	writew(htons(*(uint32_t *)&mac[4]), UNIMAC0_MAC_LSB_ADDR);

	return 0;
}

int gmac_mac_init(struct eth_device *dev)
{
	struct eth_info *eth = (struct eth_info *)(dev->priv);
	struct eth_dma *dma = &(eth->dma);

	uint32_t tmp;
	uint32_t cmdcfg;
	int chipid;

	debug("%s enter\n", __func__);

	/* Always use GMAC0 */
	printf("Using GMAC%d\n", 0);

	/* Reset AMAC0 core */
	writel(0, AMAC0_IDM_RESET_ADDR);
	tmp = readl(AMAC0_IO_CTRL_DIRECT_ADDR);
	/* Set clock */
	tmp &= ~(1 << AMAC0_IO_CTRL_CLK_250_SEL_SHIFT);
	tmp |= (1 << AMAC0_IO_CTRL_GMII_MODE_SHIFT);
	/* Set Tx clock */
	tmp &= ~(1 << AMAC0_IO_CTRL_DEST_SYNC_MODE_EN_SHIFT);
	writel(tmp, AMAC0_IO_CTRL_DIRECT_ADDR);

	/* reset gmac */
	/*
	 * As AMAC is just reset, NO need?
	 * set eth_data into loopback mode to ensure no rx traffic
	 * gmac_loopback(eth_data, TRUE);
	 * ET_TRACE(("%s gmac loopback\n", __func__));
	 * udelay(1);
	 */

	cmdcfg = readl(UNIMAC0_CMD_CFG_ADDR);
	cmdcfg &= ~(CC_TE | CC_RE | CC_RPI | CC_TAI | CC_HD | CC_ML |
		    CC_CFE | CC_RL | CC_RED | CC_PE | CC_TPI |
		    CC_PAD_EN | CC_PF);
	cmdcfg |= (CC_PROM | CC_NLC | CC_CFE);
	/* put mac in reset */
	gmac_init_reset();
	writel(cmdcfg, UNIMAC0_CMD_CFG_ADDR);
	gmac_clear_reset();

	/* enable clear MIB on read */
	reg32_set_bits(GMAC0_DEV_CTRL_ADDR, DC_MROR);
	/* PHY: set smi_master to drive mdc_clk */
	reg32_set_bits(GMAC0_PHY_CTRL_ADDR, PC_MTE);

	/* clear persistent sw intstatus */
	writel(0, GMAC0_INT_STATUS_ADDR);

	if (dma_init(dma) < 0) {
		pr_err("%s: GMAC dma_init failed\n", __func__);
		goto err_exit;
	}

	chipid = CHIPID;
	printf("%s: Chip ID: 0x%x\n", __func__, chipid);

	/* set switch bypass mode */
	tmp = readl(SWITCH_GLOBAL_CONFIG_ADDR);
	tmp |= (1 << CDRU_SWITCH_BYPASS_SWITCH_SHIFT);

	/* Switch mode */
	/* tmp &= ~(1 << CDRU_SWITCH_BYPASS_SWITCH_SHIFT); */

	writel(tmp, SWITCH_GLOBAL_CONFIG_ADDR);

	tmp = readl(CRMU_CHIP_IO_PAD_CONTROL_ADDR);
	tmp &= ~(1 << CDRU_IOMUX_FORCE_PAD_IN_SHIFT);
	writel(tmp, CRMU_CHIP_IO_PAD_CONTROL_ADDR);

	/* Set MDIO to internal GPHY */
	tmp = readl(GMAC_MII_CTRL_ADDR);
	/* Select internal MDC/MDIO bus*/
	tmp &= ~(1 << GMAC_MII_CTRL_BYP_SHIFT);
	/* select MDC/MDIO connecting to on-chip internal PHYs */
	tmp &= ~(1 << GMAC_MII_CTRL_EXT_SHIFT);
	/*
	 * give bit[6:0](MDCDIV) with required divisor to set
	 * the MDC clock frequency, 66MHZ/0x1A=2.5MHZ
	 */
	tmp |= 0x1A;

	writel(tmp, GMAC_MII_CTRL_ADDR);

	if (gmac_mii_busywait(1000)) {
		pr_err("%s: Configure MDIO: MII/MDIO busy\n", __func__);
		goto err_exit;
	}

	/* Configure GMAC0 */
	/* enable one rx interrupt per received frame */
	writel(1 << GMAC0_IRL_FRAMECOUNT_SHIFT, GMAC0_INTR_RECV_LAZY_ADDR);

	/* read command config reg */
	cmdcfg = readl(UNIMAC0_CMD_CFG_ADDR);
	/* enable 802.3x tx flow control (honor received PAUSE frames) */
	cmdcfg &= ~CC_RPI;
	/* enable promiscuous mode */
	cmdcfg |= CC_PROM;
	/* Disable loopback mode */
	cmdcfg &= ~CC_ML;
	/* set the speed */
	cmdcfg &= ~(CC_ES_MASK | CC_HD);
	/* Set to 1Gbps and full duplex by default */
	cmdcfg |= (2 << CC_ES_SHIFT);

	/* put mac in reset */
	gmac_init_reset();
	/* write register */
	writel(cmdcfg, UNIMAC0_CMD_CFG_ADDR);
	/* bring mac out of reset */
	gmac_clear_reset();

	/* set max frame lengths; account for possible vlan tag */
	writel(PKTSIZE + 32, UNIMAC0_FRM_LENGTH_ADDR);

	return 0;

err_exit:
	dma_deinit(dma);
	return -1;
}

int gmac_add(struct eth_device *dev)
{
	struct eth_info *eth = (struct eth_info *)(dev->priv);
	struct eth_dma *dma = &(eth->dma);
	void *tmp;

	/*
	 * Desc has to be 16-byte aligned. But for dcache flush it must be
	 * aligned to ARCH_DMA_MINALIGN.
	 */
	tmp = memalign(ARCH_DMA_MINALIGN, DESCP_SIZE_ALIGNED * TX_BUF_NUM);
	if (tmp == NULL) {
		printf("%s: Failed to allocate TX desc Buffer\n", __func__);
		return -1;
	}

	dma->tx_desc_aligned = (void *)tmp;
	debug("TX Descriptor Buffer: %p; length: 0x%x\n",
	      dma->tx_desc_aligned, DESCP_SIZE_ALIGNED * TX_BUF_NUM);

	tmp = memalign(ARCH_DMA_MINALIGN, TX_BUF_SIZE_ALIGNED * TX_BUF_NUM);
	if (tmp == NULL) {
		printf("%s: Failed to allocate TX Data Buffer\n", __func__);
		free(dma->tx_desc_aligned);
		return -1;
	}
	dma->tx_buf = (uint8_t *)tmp;
	debug("TX Data Buffer: %p; length: 0x%x\n",
	      dma->tx_buf, TX_BUF_SIZE_ALIGNED * TX_BUF_NUM);

	/* Desc has to be 16-byte aligned */
	tmp = memalign(ARCH_DMA_MINALIGN, DESCP_SIZE_ALIGNED * RX_BUF_NUM);
	if (tmp == NULL) {
		printf("%s: Failed to allocate RX Descriptor\n", __func__);
		free(dma->tx_desc_aligned);
		free(dma->tx_buf);
		return -1;
	}
	dma->rx_desc_aligned = (void *)tmp;
	debug("RX Descriptor Buffer: %p, length: 0x%x\n",
	      dma->rx_desc_aligned, DESCP_SIZE_ALIGNED * RX_BUF_NUM);

	tmp = memalign(ARCH_DMA_MINALIGN, RX_BUF_SIZE_ALIGNED * RX_BUF_NUM);
	if (tmp == NULL) {
		printf("%s: Failed to allocate RX Data Buffer\n", __func__);
		free(dma->tx_desc_aligned);
		free(dma->tx_buf);
		free(dma->rx_desc_aligned);
		return -1;
	}
	dma->rx_buf = (uint8_t *)tmp;
	debug("RX Data Buffer: %p; length: 0x%x\n",
	      dma->rx_buf, RX_BUF_SIZE_ALIGNED * RX_BUF_NUM);

	g_dmactrlflags = 0;

	eth->phy_interface = PHY_INTERFACE_MODE_GMII;

	dma->tx_packet = gmac_tx_packet;
	dma->check_tx_done = gmac_check_tx_done;

	dma->check_rx_done = gmac_check_rx_done;

	dma->enable_dma = gmac_enable_dma;
	dma->disable_dma = gmac_disable_dma;

	eth->miiphy_read = gmac_miiphy_read;
	eth->miiphy_write = gmac_miiphy_write;

	eth->mac_init = gmac_mac_init;
	eth->disable_mac = gmac_disable;
	eth->enable_mac = gmac_enable;
	eth->set_mac_addr = gmac_set_mac_addr;
	eth->set_mac_speed = gmac_set_speed;

	return 0;
}
