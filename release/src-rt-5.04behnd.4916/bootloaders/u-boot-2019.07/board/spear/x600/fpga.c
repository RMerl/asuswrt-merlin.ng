// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2012 Stefan Roese <sr@denx.de>
 */

#include <common.h>
#include <spartan3.h>
#include <command.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/arch/spr_misc.h>
#include <asm/arch/spr_ssp.h>

/*
 * FPGA program pin configuration on X600:
 *
 * Only PROG and DONE are connected to GPIOs. INIT is not connected to the
 * SoC at all. And CLOCK and DATA are connected to the SSP2 port. We use
 * 16bit serial writes via this SSP port to write the data bits into the
 * FPGA.
 */
#define CONFIG_SYS_FPGA_PROG		2
#define CONFIG_SYS_FPGA_DONE		3

/*
 * Set the active-low FPGA reset signal.
 */
static void fpga_reset(int assert)
{
	/*
	 * On x600 we have no means to toggle the FPGA reset signal
	 */
	debug("%s:%d: RESET (%d)\n", __func__, __LINE__, assert);
}

/*
 * Set the FPGA's active-low SelectMap program line to the specified level
 */
static int fpga_pgm_fn(int assert, int flush, int cookie)
{
	debug("%s:%d: FPGA PROG (%d)\n", __func__, __LINE__, assert);

	gpio_set_value(CONFIG_SYS_FPGA_PROG, assert);

	return assert;
}

/*
 * Test the state of the active-low FPGA INIT line.  Return 1 on INIT
 * asserted (low).
 */
static int fpga_init_fn(int cookie)
{
	static int state;

	debug("%s:%d: init (state=%d)\n", __func__, __LINE__, state);

	/*
	 * On x600, the FPGA INIT signal is not connected to the SoC.
	 * We can't read the INIT status. Let's return the "correct"
	 * INIT signal state generated via a local state-machine.
	 */
	if (++state == 1) {
		return 1;
	} else {
		state = 0;
		return 0;
	}
}

/*
 * Test the state of the active-high FPGA DONE pin
 */
static int fpga_done_fn(int cookie)
{
	struct ssp_regs *ssp = (struct ssp_regs *)CONFIG_SSP2_BASE;

	/*
	 * Wait for Tx-FIFO to become empty before looking for DONE
	 */
	while (!(readl(&ssp->sspsr) & SSPSR_TFE))
		;

	if (gpio_get_value(CONFIG_SYS_FPGA_DONE))
		return 1;
	else
		return 0;
}

/*
 * FPGA pre-configuration function. Just make sure that
 * FPGA reset is asserted to keep the FPGA from starting up after
 * configuration.
 */
static int fpga_pre_config_fn(int cookie)
{
	debug("%s:%d: FPGA pre-configuration\n", __func__, __LINE__);
	fpga_reset(true);

	return 0;
}

/*
 * FPGA post configuration function. Blip the FPGA reset line and then see if
 * the FPGA appears to be running.
 */
static int fpga_post_config_fn(int cookie)
{
	int rc = 0;

	debug("%s:%d: FPGA post configuration\n", __func__, __LINE__);

	fpga_reset(true);
	udelay(100);
	fpga_reset(false);
	udelay(100);

	return rc;
}

static int fpga_clk_fn(int assert_clk, int flush, int cookie)
{
	/*
	 * No dedicated clock signal on x600 (data & clock generated)
	 * in SSP interface. So we don't have to do anything here.
	 */
	return assert_clk;
}

static int fpga_wr_fn(int assert_write, int flush, int cookie)
{
	struct ssp_regs *ssp = (struct ssp_regs *)CONFIG_SSP2_BASE;
	static int count;
	static u16 data;

	/*
	 * First collect 16 bits of data
	 */
	data = data << 1;
	if (assert_write)
		data |= 1;

	/*
	 * If 16 bits are not available, return for more bits
	 */
	count++;
	if (count != 16)
		return assert_write;

	count = 0;

	/*
	 * Wait for Tx-FIFO to become ready
	 */
	while (!(readl(&ssp->sspsr) & SSPSR_TNF))
		;

	/* Send 16 bits to FPGA via SSP bus */
	writel(data, &ssp->sspdr);

	return assert_write;
}

static xilinx_spartan3_slave_serial_fns x600_fpga_fns = {
	fpga_pre_config_fn,
	fpga_pgm_fn,
	fpga_clk_fn,
	fpga_init_fn,
	fpga_done_fn,
	fpga_wr_fn,
	fpga_post_config_fn,
};

static xilinx_desc fpga[CONFIG_FPGA_COUNT] = {
	XILINX_XC3S1200E_DESC(slave_serial, &x600_fpga_fns, 0)
};

/*
 * Initialize the SelectMap interface.  We assume that the mode and the
 * initial state of all of the port pins have already been set!
 */
static void fpga_serialslave_init(void)
{
	debug("%s:%d: Initialize serial slave interface\n", __func__, __LINE__);
	fpga_pgm_fn(false, false, 0);	/* make sure program pin is inactive */
}

static int expi_setup(int freq)
{
	struct misc_regs *misc = (struct misc_regs *)CONFIG_SPEAR_MISCBASE;
	int pll2_m, pll2_n, pll2_p, expi_x, expi_y;

	pll2_m = (freq * 2) / 1000;
	pll2_n = 15;
	pll2_p = 1;
	expi_x = 1;
	expi_y = 2;

	/*
	 * Disable reset, Low compression, Disable retiming, Enable Expi,
	 * Enable soft reset, DMA, PLL2, Internal
	 */
	writel(EXPI_CLK_CFG_LOW_COMPR | EXPI_CLK_CFG_CLK_EN | EXPI_CLK_CFG_RST |
	       EXPI_CLK_SYNT_EN | EXPI_CLK_CFG_SEL_PLL2 |
	       EXPI_CLK_CFG_INT_CLK_EN | (expi_y << 16) | (expi_x << 24),
	       &misc->expi_clk_cfg);

	/*
	 * 6 uA, Internal feedback, 1st order, Non-dithered, Sample Parameters,
	 * Enable PLL2, Disable reset
	 */
	writel((pll2_m << 24) | (pll2_p << 8) | (pll2_n), &misc->pll2_frq);
	writel(PLL2_CNTL_6UA | PLL2_CNTL_SAMPLE | PLL2_CNTL_ENABLE |
	       PLL2_CNTL_RESETN | PLL2_CNTL_LOCK, &misc->pll2_cntl);

	/*
	 * Disable soft reset
	 */
	clrbits_le32(&misc->expi_clk_cfg, EXPI_CLK_CFG_RST);

	return 0;
}

/*
 * Initialize the fpga
 */
int x600_init_fpga(void)
{
	struct ssp_regs *ssp = (struct ssp_regs *)CONFIG_SSP2_BASE;
	struct misc_regs *misc = (struct misc_regs *)CONFIG_SPEAR_MISCBASE;

	/* Enable SSP2 clock */
	writel(readl(&misc->periph1_clken) | MISC_SSP2ENB | MISC_GPIO4ENB,
	       &misc->periph1_clken);

	/* Set EXPI clock to 45 MHz */
	expi_setup(45000);

	/* Configure GPIO directions */
	gpio_direction_output(CONFIG_SYS_FPGA_PROG, 0);
	gpio_direction_input(CONFIG_SYS_FPGA_DONE);

	writel(SSPCR0_DSS_16BITS, &ssp->sspcr0);
	writel(SSPCR1_SSE, &ssp->sspcr1);

	/*
	 * Set lowest prescale divisor value (CPSDVSR) of 2 for max download
	 * speed.
	 *
	 * Actual data clock rate is: 80MHz / (CPSDVSR * (SCR + 1))
	 * With CPSDVSR at 2 and SCR at 0, the maximume clock rate is 40MHz.
	 */
	writel(2, &ssp->sspcpsr);

	fpga_init();
	fpga_serialslave_init();

	debug("%s:%d: Adding fpga 0\n", __func__, __LINE__);
	fpga_add(fpga_xilinx, &fpga[0]);

	return 0;
}
