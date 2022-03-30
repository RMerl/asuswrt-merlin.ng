// SPDX-License-Identifier: GPL-2.0+
/*
 * LPC32xx SSP interface (SPI mode)
 *
 * (C) Copyright 2014  DENX Software Engineering GmbH
 * Written-by: Albert ARIBAUD <albert.aribaud@3adev.fr>
 */

#include <common.h>
#include <linux/compat.h>
#include <asm/io.h>
#include <malloc.h>
#include <spi.h>
#include <asm/arch/clk.h>

/* SSP chip registers */
struct ssp_regs {
	u32 cr0;
	u32 cr1;
	u32 data;
	u32 sr;
	u32 cpsr;
	u32 imsc;
	u32 ris;
	u32 mis;
	u32 icr;
	u32 dmacr;
};

/* CR1 register defines  */
#define SSP_CR1_SSP_ENABLE 0x0002

/* SR register defines  */
#define SSP_SR_TNF 0x0002
/* SSP status RX FIFO not empty bit */
#define SSP_SR_RNE 0x0004

/* lpc32xx spi slave */
struct lpc32xx_spi_slave {
	struct spi_slave slave;
	struct ssp_regs *regs;
};

static inline struct lpc32xx_spi_slave *to_lpc32xx_spi_slave(
	struct spi_slave *slave)
{
	return container_of(slave, struct lpc32xx_spi_slave, slave);
}

/* the following is called in sequence by do_spi_xfer() */

struct spi_slave *spi_setup_slave(uint bus, uint cs, uint max_hz, uint mode)
{
	struct lpc32xx_spi_slave *lslave;

	/* we only set up SSP0 for now, so ignore bus */

	if (mode & SPI_3WIRE) {
		pr_err("3-wire mode not supported");
		return NULL;
	}

	if (mode & SPI_SLAVE) {
		pr_err("slave mode not supported\n");
		return NULL;
	}

	if (mode & SPI_PREAMBLE) {
		pr_err("preamble byte skipping not supported\n");
		return NULL;
	}

	lslave = spi_alloc_slave(struct lpc32xx_spi_slave, bus, cs);
	if (!lslave) {
		printf("SPI_error: Fail to allocate lpc32xx_spi_slave\n");
		return NULL;
	}

	lslave->regs = (struct ssp_regs *)SSP0_BASE;

	/*
	 * 8 bit frame, SPI fmt, 500kbps -> clock divider is 26.
	 * Set SCR to 0 and CPSDVSR to 26.
	 */

	writel(0x7, &lslave->regs->cr0); /* 8-bit chunks, SPI, 1 clk/bit */
	writel(26, &lslave->regs->cpsr); /* SSP clock = HCLK/26 = 500kbps */
	writel(0, &lslave->regs->imsc); /* do not raise any interrupts */
	writel(0, &lslave->regs->icr); /* clear any pending interrupt */
	writel(0, &lslave->regs->dmacr); /* do not do DMAs */
	writel(SSP_CR1_SSP_ENABLE, &lslave->regs->cr1); /* enable SSP0 */
	return &lslave->slave;
}

void spi_free_slave(struct spi_slave *slave)
{
	struct lpc32xx_spi_slave *lslave = to_lpc32xx_spi_slave(slave);

	debug("(lpc32xx) spi_free_slave: 0x%08x\n", (u32)lslave);
	free(lslave);
}

int spi_claim_bus(struct spi_slave *slave)
{
	/* only one bus and slave so far, always available */
	return 0;
}

int spi_xfer(struct spi_slave *slave, unsigned int bitlen,
	const void *dout, void *din, unsigned long flags)
{
	struct lpc32xx_spi_slave *lslave = to_lpc32xx_spi_slave(slave);
	int bytelen = bitlen >> 3;
	int idx_out = 0;
	int idx_in = 0;
	int start_time;

	start_time = get_timer(0);
	while ((idx_out < bytelen) || (idx_in < bytelen)) {
		int status = readl(&lslave->regs->sr);
		if ((idx_out < bytelen) && (status & SSP_SR_TNF))
			writel(((u8 *)dout)[idx_out++], &lslave->regs->data);
		if ((idx_in < bytelen) && (status & SSP_SR_RNE))
			((u8 *)din)[idx_in++] = readl(&lslave->regs->data);
		if (get_timer(start_time) >= CONFIG_LPC32XX_SSP_TIMEOUT)
			return -1;
	}
	return 0;
}

void spi_release_bus(struct spi_slave *slave)
{
	/* do nothing */
}
