// SPDX-License-Identifier: GPL-2.0
/*
 * Xilinx Multirate Ethernet MAC(MRMAC) driver
 *
 * Author(s):   Ashok Reddy Soma <ashok.reddy.soma@xilinx.com>
 *              Michal Simek <michal.simek@amd.com>
 *
 * Copyright (C) 2021 Xilinx, Inc. All rights reserved.
 */

#include <config.h>
#include <common.h>
#include <cpu_func.h>
#include <dm.h>
#include <log.h>
#include <net.h>
#include <malloc.h>
#include <wait_bit.h>
#include <asm/io.h>
#include <linux/delay.h>
#include <linux/ethtool.h>
#include "xilinx_axi_mrmac.h"

static void axi_mrmac_dma_write(struct mcdma_bd *bd, u32 *desc)
{
	if (IS_ENABLED(CONFIG_PHYS_64BIT))
		writeq((unsigned long)bd, desc);
	else
		writel((uintptr_t)bd, desc);
}

/**
 * axi_mrmac_ethernet_init - MRMAC init function
 * @priv:	MRMAC private structure
 *
 * Return:	0 on success, negative value on errors
 *
 * This function is called to reset and initialize MRMAC core. This is
 * typically called during initialization. It does a reset of MRMAC Rx/Tx
 * channels and Rx/Tx SERDES. It configures MRMAC speed based on mrmac_rate
 * which is read from DT. This function waits for block lock bit to get set,
 * if it is not set within 100ms time returns a timeout error.
 */
static int axi_mrmac_ethernet_init(struct axi_mrmac_priv *priv)
{
	struct mrmac_regs *regs = priv->iobase;
	u32 reg;
	u32 ret;

	/* Perform all the RESET's required */
	setbits_le32(&regs->reset, MRMAC_RX_SERDES_RST_MASK | MRMAC_RX_RST_MASK
		     | MRMAC_TX_SERDES_RST_MASK | MRMAC_TX_RST_MASK);

	mdelay(MRMAC_RESET_DELAY);

	/* Configure Mode register */
	reg = readl(&regs->mode);

	log_debug("Configuring MRMAC speed to %d\n", priv->mrmac_rate);

	if (priv->mrmac_rate == SPEED_25000) {
		reg &= ~MRMAC_CTL_RATE_CFG_MASK;
		reg |= MRMAC_CTL_DATA_RATE_25G;
		reg |= (MRMAC_CTL_AXIS_CFG_25G_IND << MRMAC_CTL_AXIS_CFG_SHIFT);
		reg |= (MRMAC_CTL_SERDES_WIDTH_25G <<
			MRMAC_CTL_SERDES_WIDTH_SHIFT);
	} else {
		reg &= ~MRMAC_CTL_RATE_CFG_MASK;
		reg |= MRMAC_CTL_DATA_RATE_10G;
		reg |= (MRMAC_CTL_AXIS_CFG_10G_IND << MRMAC_CTL_AXIS_CFG_SHIFT);
		reg |= (MRMAC_CTL_SERDES_WIDTH_10G <<
			MRMAC_CTL_SERDES_WIDTH_SHIFT);
	}

	/* For tick reg */
	reg |= MRMAC_CTL_PM_TICK_MASK;
	writel(reg, &regs->mode);

	clrbits_le32(&regs->reset, MRMAC_RX_SERDES_RST_MASK | MRMAC_RX_RST_MASK
		     | MRMAC_TX_SERDES_RST_MASK | MRMAC_TX_RST_MASK);

	mdelay(MRMAC_RESET_DELAY);

	/* Setup MRMAC hardware options */
	setbits_le32(&regs->rx_config, MRMAC_RX_DEL_FCS_MASK);
	setbits_le32(&regs->tx_config, MRMAC_TX_INS_FCS_MASK);
	setbits_le32(&regs->tx_config, MRMAC_TX_EN_MASK);
	setbits_le32(&regs->rx_config, MRMAC_RX_EN_MASK);

	/* Check for block lock bit to be set. This ensures that
	 * MRMAC ethernet IP is functioning normally.
	 */
	writel(MRMAC_STS_ALL_MASK, (phys_addr_t)priv->iobase +
		MRMAC_TX_STS_OFFSET);
	writel(MRMAC_STS_ALL_MASK, (phys_addr_t)priv->iobase +
		MRMAC_RX_STS_OFFSET);
	writel(MRMAC_STS_ALL_MASK, (phys_addr_t)priv->iobase +
		MRMAC_STATRX_BLKLCK_OFFSET);

	ret = wait_for_bit_le32((u32 *)((phys_addr_t)priv->iobase +
				MRMAC_STATRX_BLKLCK_OFFSET),
				MRMAC_RX_BLKLCK_MASK, true,
				MRMAC_BLKLCK_TIMEOUT, true);
	if (ret) {
		log_warning("Error: MRMAC block lock not complete!\n");
		return -EIO;
	}

	writel(MRMAC_TICK_TRIGGER, &regs->tick_reg);

	return 0;
}

/**
 * axi_mcdma_init - Reset MCDMA engine
 * @priv:	MRMAC private structure
 *
 * Return:	0 on success, negative value on timeouts
 *
 * This function is called to reset and initialize MCDMA engine
 */
static int axi_mcdma_init(struct axi_mrmac_priv *priv)
{
	u32 ret;

	/* Reset the engine so the hardware starts from a known state */
	writel(XMCDMA_CR_RESET, &priv->mm2s_cmn->control);
	writel(XMCDMA_CR_RESET, &priv->s2mm_cmn->control);

	/* Check Tx/Rx MCDMA.RST. Reset is done when the reset bit is low */
	ret = wait_for_bit_le32(&priv->mm2s_cmn->control, XMCDMA_CR_RESET,
				false, MRMAC_DMARST_TIMEOUT, true);
	if (ret) {
		log_warning("Tx MCDMA reset Timeout\n");
		return -ETIMEDOUT;
	}

	ret = wait_for_bit_le32(&priv->s2mm_cmn->control, XMCDMA_CR_RESET,
				false, MRMAC_DMARST_TIMEOUT, true);
	if (ret) {
		log_warning("Rx MCDMA reset Timeout\n");
		return -ETIMEDOUT;
	}

	/* Enable channel 1 for Tx and Rx */
	writel(XMCDMA_CHANNEL_1, &priv->mm2s_cmn->chen);
	writel(XMCDMA_CHANNEL_1, &priv->s2mm_cmn->chen);

	return 0;
}

/**
 * axi_mrmac_start - MRMAC start
 * @dev:	udevice structure
 *
 * Return:	0 on success, negative value on errors
 *
 * This is a initialization function of MRMAC. Call MCDMA initialization
 * function and setup Rx buffer descriptors for starting reception of packets.
 * Enable Tx and Rx channels and trigger Rx channel fetch.
 */
static int axi_mrmac_start(struct udevice *dev)
{
	struct axi_mrmac_priv *priv = dev_get_priv(dev);
	struct mrmac_regs *regs = priv->iobase;

	/*
	 * Initialize MCDMA engine. MCDMA engine must be initialized before
	 * MRMAC. During MCDMA engine initialization, MCDMA hardware is reset,
	 * since MCDMA reset line is connected to MRMAC, this would ensure a
	 * reset of MRMAC.
	 */
	axi_mcdma_init(priv);

	/* Initialize MRMAC hardware */
	if (axi_mrmac_ethernet_init(priv))
		return -EIO;

	/* Disable all Rx interrupts before RxBD space setup */
	clrbits_le32(&priv->mcdma_rx->control, XMCDMA_IRQ_ALL_MASK);

	/* Update current descriptor */
	axi_mrmac_dma_write(priv->rx_bd[0], &priv->mcdma_rx->current);

	/* Setup Rx BD. MRMAC needs atleast two descriptors */
	memset(priv->rx_bd[0], 0, RX_BD_TOTAL_SIZE);

	priv->rx_bd[0]->next_desc = lower_32_bits((u64)priv->rx_bd[1]);
	priv->rx_bd[0]->buf_addr = lower_32_bits((u64)net_rx_packets[0]);

	priv->rx_bd[1]->next_desc = lower_32_bits((u64)priv->rx_bd[0]);
	priv->rx_bd[1]->buf_addr = lower_32_bits((u64)net_rx_packets[1]);

	if (IS_ENABLED(CONFIG_PHYS_64BIT)) {
		priv->rx_bd[0]->next_desc_msb = upper_32_bits((u64)priv->rx_bd[1]);
		priv->rx_bd[0]->buf_addr_msb = upper_32_bits((u64)net_rx_packets[0]);

		priv->rx_bd[1]->next_desc_msb = upper_32_bits((u64)priv->rx_bd[0]);
		priv->rx_bd[1]->buf_addr_msb = upper_32_bits((u64)net_rx_packets[1]);
	}

	priv->rx_bd[0]->cntrl = PKTSIZE_ALIGN;
	priv->rx_bd[1]->cntrl = PKTSIZE_ALIGN;

	/* Flush the last BD so DMA core could see the updates */
	flush_cache((phys_addr_t)priv->rx_bd[0], RX_BD_TOTAL_SIZE);

	/* It is necessary to flush rx buffers because if you don't do it
	 * then cache can contain uninitialized data
	 */
	flush_cache((phys_addr_t)priv->rx_bd[0]->buf_addr, RX_BUFF_TOTAL_SIZE);

	/* Start the hardware */
	setbits_le32(&priv->s2mm_cmn->control, XMCDMA_CR_RUNSTOP_MASK);
	setbits_le32(&priv->mm2s_cmn->control, XMCDMA_CR_RUNSTOP_MASK);
	setbits_le32(&priv->mcdma_rx->control, XMCDMA_IRQ_ALL_MASK);

	/* Channel fetch */
	setbits_le32(&priv->mcdma_rx->control, XMCDMA_CR_RUNSTOP_MASK);

	/* Update tail descriptor. Now it's ready to receive data */
	axi_mrmac_dma_write(priv->rx_bd[1], &priv->mcdma_rx->tail);

	/* Enable Tx */
	setbits_le32(&regs->tx_config, MRMAC_TX_EN_MASK);

	/* Enable Rx */
	setbits_le32(&regs->rx_config, MRMAC_RX_EN_MASK);

	return 0;
}

/**
 * axi_mrmac_send - MRMAC Tx function
 * @dev:	udevice structure
 * @ptr:	pointer to Tx buffer
 * @len:	transfer length
 *
 * Return:	0 on success, negative value on errors
 *
 * This is a Tx send function of MRMAC. Setup Tx buffer descriptors and trigger
 * transfer. Wait till the data is transferred.
 */
static int axi_mrmac_send(struct udevice *dev, void *ptr, int len)
{
	struct axi_mrmac_priv *priv = dev_get_priv(dev);
	u32 ret;

#ifdef DEBUG
	print_buffer(ptr, ptr, 1, len, 16);
#endif
	if (len > PKTSIZE_ALIGN)
		len = PKTSIZE_ALIGN;

	/* If size is less than min packet size, pad to min size */
	if (len < MIN_PKT_SIZE) {
		memset(priv->txminframe, 0, MIN_PKT_SIZE);
		memcpy(priv->txminframe, ptr, len);
		len = MIN_PKT_SIZE;
		ptr = priv->txminframe;
	}

	writel(XMCDMA_IRQ_ALL_MASK, &priv->mcdma_tx->status);

	clrbits_le32(&priv->mcdma_tx->control, XMCDMA_CR_RUNSTOP_MASK);

	/* Flush packet to main memory to be trasfered by DMA */
	flush_cache((phys_addr_t)ptr, len);

	/* Setup Tx BD. MRMAC needs atleast two descriptors */
	memset(priv->tx_bd[0], 0, TX_BD_TOTAL_SIZE);

	priv->tx_bd[0]->next_desc = lower_32_bits((u64)priv->tx_bd[1]);
	priv->tx_bd[0]->buf_addr = lower_32_bits((u64)ptr);

	/* At the end of the ring, link the last BD back to the top */
	priv->tx_bd[1]->next_desc = lower_32_bits((u64)priv->tx_bd[0]);
	priv->tx_bd[1]->buf_addr = lower_32_bits((u64)ptr + len / 2);

	if (IS_ENABLED(CONFIG_PHYS_64BIT)) {
		priv->tx_bd[0]->next_desc_msb = upper_32_bits((u64)priv->tx_bd[1]);
		priv->tx_bd[0]->buf_addr_msb = upper_32_bits((u64)ptr);

		priv->tx_bd[1]->next_desc_msb = upper_32_bits((u64)priv->tx_bd[0]);
		priv->tx_bd[1]->buf_addr_msb = upper_32_bits((u64)ptr + len / 2);
	}

	/* Split Tx data in to half and send in two descriptors */
	priv->tx_bd[0]->cntrl = (len / 2) | XMCDMA_BD_CTRL_TXSOF_MASK;
	priv->tx_bd[1]->cntrl = (len - len / 2) | XMCDMA_BD_CTRL_TXEOF_MASK;

	/* Flush the last BD so DMA core could see the updates */
	flush_cache((phys_addr_t)priv->tx_bd[0], TX_BD_TOTAL_SIZE);

	if (readl(&priv->mcdma_tx->status) & XMCDMA_CH_IDLE) {
		axi_mrmac_dma_write(priv->tx_bd[0], &priv->mcdma_tx->current);
		/* Channel fetch */
		setbits_le32(&priv->mcdma_tx->control, XMCDMA_CR_RUNSTOP_MASK);
	} else {
		log_warning("Error: current desc is not updated\n");
		return -EIO;
	}

	setbits_le32(&priv->mcdma_tx->control, XMCDMA_IRQ_ALL_MASK);

	/* Start transfer */
	axi_mrmac_dma_write(priv->tx_bd[1], &priv->mcdma_tx->tail);

	/* Wait for transmission to complete */
	ret = wait_for_bit_le32(&priv->mcdma_tx->status, XMCDMA_IRQ_IOC_MASK,
				true, 1, true);
	if (ret) {
		log_warning("%s: Timeout\n", __func__);
		return -ETIMEDOUT;
	}

	/* Clear status */
	priv->tx_bd[0]->sband_stats = 0;
	priv->tx_bd[1]->sband_stats = 0;

	log_debug("Sending complete\n");

	return 0;
}

static bool isrxready(struct axi_mrmac_priv *priv)
{
	u32 status;

	/* Read pending interrupts */
	status = readl(&priv->mcdma_rx->status);

	/* Acknowledge pending interrupts */
	writel(status & XMCDMA_IRQ_ALL_MASK, &priv->mcdma_rx->status);

	/*
	 * If Reception done interrupt is asserted, call Rx call back function
	 * to handle the processed BDs and then raise the according flag.
	 */
	if (status & (XMCDMA_IRQ_IOC_MASK | XMCDMA_IRQ_DELAY_MASK))
		return 1;

	return 0;
}

/**
 * axi_mrmac_recv - MRMAC Rx function
 * @dev:	udevice structure
 * @flags:	flags from network stack
 * @packetp	pointer to received data
 *
 * Return:	received data length on success, negative value on errors
 *
 * This is a Rx function of MRMAC. Check if any data is received on MCDMA.
 * Copy buffer pointer to packetp and return received data length.
 */
static int axi_mrmac_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct axi_mrmac_priv *priv = dev_get_priv(dev);
	u32 rx_bd_end;
	u32 length;

	/* Wait for an incoming packet */
	if (!isrxready(priv))
		return -EAGAIN;

	/* Clear all interrupts */
	writel(XMCDMA_IRQ_ALL_MASK, &priv->mcdma_rx->status);

	/* Disable IRQ for a moment till packet is handled */
	clrbits_le32(&priv->mcdma_rx->control, XMCDMA_IRQ_ALL_MASK);

	/* Disable channel fetch */
	clrbits_le32(&priv->mcdma_rx->control, XMCDMA_CR_RUNSTOP_MASK);

	rx_bd_end = (ulong)priv->rx_bd[0] + roundup(RX_BD_TOTAL_SIZE,
						    ARCH_DMA_MINALIGN);
	/* Invalidate Rx descriptors to see proper Rx length */
	invalidate_dcache_range((phys_addr_t)priv->rx_bd[0], rx_bd_end);

	length = priv->rx_bd[0]->status & XMCDMA_BD_STS_ACTUAL_LEN_MASK;
	*packetp = (uchar *)(ulong)priv->rx_bd[0]->buf_addr;

	if (!length) {
		length = priv->rx_bd[1]->status & XMCDMA_BD_STS_ACTUAL_LEN_MASK;
		*packetp = (uchar *)(ulong)priv->rx_bd[1]->buf_addr;
	}

#ifdef DEBUG
	print_buffer(*packetp, *packetp, 1, length, 16);
#endif
	/* Clear status */
	priv->rx_bd[0]->status = 0;
	priv->rx_bd[1]->status = 0;

	return length;
}

/**
 * axi_mrmac_free_pkt - MRMAC free packet function
 * @dev:	udevice structure
 * @packet:	receive buffer pointer
 * @length	received data length
 *
 * Return:	0 on success, negative value on errors
 *
 * This is Rx free packet function of MRMAC. Prepare MRMAC for reception of
 * data again. Invalidate previous data from Rx buffers and set Rx buffer
 * descriptors. Trigger reception by updating tail descriptor.
 */
static int axi_mrmac_free_pkt(struct udevice *dev, uchar *packet, int length)
{
	struct axi_mrmac_priv *priv = dev_get_priv(dev);

#ifdef DEBUG
	/* It is useful to clear buffer to be sure that it is consistent */
	memset(priv->rx_bd[0]->buf_addr, 0, RX_BUFF_TOTAL_SIZE);
#endif
	/* Disable all Rx interrupts before RxBD space setup */
	clrbits_le32(&priv->mcdma_rx->control, XMCDMA_IRQ_ALL_MASK);

	/* Disable channel fetch */
	clrbits_le32(&priv->mcdma_rx->control, XMCDMA_CR_RUNSTOP_MASK);

	/* Update current descriptor */
	axi_mrmac_dma_write(priv->rx_bd[0], &priv->mcdma_rx->current);

	/* Write bd to HW */
	flush_cache((phys_addr_t)priv->rx_bd[0], RX_BD_TOTAL_SIZE);

	/* It is necessary to flush rx buffers because if you don't do it
	 * then cache will contain previous packet
	 */
	flush_cache((phys_addr_t)priv->rx_bd[0]->buf_addr, RX_BUFF_TOTAL_SIZE);

	/* Enable all IRQ */
	setbits_le32(&priv->mcdma_rx->control, XMCDMA_IRQ_ALL_MASK);

	/* Channel fetch */
	setbits_le32(&priv->mcdma_rx->control, XMCDMA_CR_RUNSTOP_MASK);

	/* Update tail descriptor. Now it's ready to receive data */
	axi_mrmac_dma_write(priv->rx_bd[1], &priv->mcdma_rx->tail);

	log_debug("Rx completed, framelength = %x\n", length);

	return 0;
}

/**
 * axi_mrmac_stop - Stop MCDMA transfers
 * @dev:	udevice structure
 *
 * Return:	0 on success, negative value on errors
 *
 * Stop MCDMA engine for both Tx and Rx transfers.
 */
static void axi_mrmac_stop(struct udevice *dev)
{
	struct axi_mrmac_priv *priv = dev_get_priv(dev);

	/* Stop the hardware */
	clrbits_le32(&priv->mcdma_tx->control, XMCDMA_CR_RUNSTOP_MASK);
	clrbits_le32(&priv->mcdma_rx->control, XMCDMA_CR_RUNSTOP_MASK);

	log_debug("Halted\n");
}

static int axi_mrmac_probe(struct udevice *dev)
{
	struct axi_mrmac_plat *plat = dev_get_plat(dev);
	struct eth_pdata *pdata = &plat->eth_pdata;
	struct axi_mrmac_priv *priv = dev_get_priv(dev);

	priv->iobase = (struct mrmac_regs *)pdata->iobase;

	priv->mm2s_cmn = plat->mm2s_cmn;
	priv->mcdma_tx = (struct mcdma_chan_reg *)((phys_addr_t)priv->mm2s_cmn
						   + XMCDMA_CHAN_OFFSET);
	priv->s2mm_cmn = (struct mcdma_common_regs *)((phys_addr_t)priv->mm2s_cmn
						      + XMCDMA_RX_OFFSET);
	priv->mcdma_rx = (struct mcdma_chan_reg *)((phys_addr_t)priv->s2mm_cmn
						   + XMCDMA_CHAN_OFFSET);
	priv->mrmac_rate = plat->mrmac_rate;

	/* Align buffers to ARCH_DMA_MINALIGN */
	priv->tx_bd[0] = memalign(ARCH_DMA_MINALIGN, TX_BD_TOTAL_SIZE);
	priv->tx_bd[1] = (struct mcdma_bd *)((ulong)priv->tx_bd[0] +
					     sizeof(struct mcdma_bd));

	priv->rx_bd[0] = memalign(ARCH_DMA_MINALIGN, RX_BD_TOTAL_SIZE);
	priv->rx_bd[1] = (struct mcdma_bd *)((ulong)priv->rx_bd[0] +
					     sizeof(struct mcdma_bd));

	priv->txminframe = memalign(ARCH_DMA_MINALIGN, MIN_PKT_SIZE);

	return 0;
}

static int axi_mrmac_remove(struct udevice *dev)
{
	struct axi_mrmac_priv *priv = dev_get_priv(dev);

	/* Free buffer descriptors */
	free(priv->tx_bd[0]);
	free(priv->rx_bd[0]);
	free(priv->txminframe);

	return 0;
}

static int axi_mrmac_of_to_plat(struct udevice *dev)
{
	struct axi_mrmac_plat *plat = dev_get_plat(dev);
	struct eth_pdata *pdata = &plat->eth_pdata;
	struct ofnode_phandle_args phandle_args;
	int ret = 0;

	pdata->iobase = dev_read_addr(dev);

	ret = dev_read_phandle_with_args(dev, "axistream-connected", NULL, 0, 0,
					 &phandle_args);
	if (ret) {
		log_debug("axistream not found\n");
		return -EINVAL;
	}

	plat->mm2s_cmn = (struct mcdma_common_regs *)ofnode_read_u64_default
						(phandle_args.node, "reg", -1);
	if (!plat->mm2s_cmn) {
		log_warning("MRMAC dma register space not found\n");
		return -EINVAL;
	}

	/* Set default MRMAC rate to 10000 */
	plat->mrmac_rate = dev_read_u32_default(dev, "xlnx,mrmac-rate", 10000);

	return 0;
}

static const struct eth_ops axi_mrmac_ops = {
	.start			= axi_mrmac_start,
	.send			= axi_mrmac_send,
	.recv			= axi_mrmac_recv,
	.free_pkt		= axi_mrmac_free_pkt,
	.stop			= axi_mrmac_stop,
};

static const struct udevice_id axi_mrmac_ids[] = {
	{ .compatible = "xlnx,mrmac-ethernet-1.0" },
	{ }
};

U_BOOT_DRIVER(axi_mrmac) = {
	.name   = "axi_mrmac",
	.id     = UCLASS_ETH,
	.of_match = axi_mrmac_ids,
	.of_to_plat = axi_mrmac_of_to_plat,
	.probe  = axi_mrmac_probe,
	.remove = axi_mrmac_remove,
	.ops    = &axi_mrmac_ops,
	.priv_auto = sizeof(struct axi_mrmac_priv),
	.plat_auto = sizeof(struct axi_mrmac_plat),
};
