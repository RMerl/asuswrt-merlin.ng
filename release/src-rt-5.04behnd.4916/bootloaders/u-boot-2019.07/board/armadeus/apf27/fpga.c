// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2002-2013
 * Eric Jarrige <eric.jarrige@armadeus.org>
 *
 * based on the files by
 * Rich Ireland, Enterasys Networks, rireland@enterasys.com
 * and
 * Keith Outwater, keith_outwater@mvis.com
 */
#include <common.h>

#include <asm/arch/imx-regs.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <command.h>
#include <config.h>
#include "fpga.h"
#include <spartan3.h>
#include "apf27.h"

/*
 * Note that these are pointers to code that is in Flash.  They will be
 * relocated at runtime.
 * Spartan2 code is used to download our Spartan 3 :) code is compatible.
 * Just take care about the file size
 */
xilinx_spartan3_slave_parallel_fns fpga_fns = {
	fpga_pre_fn,
	fpga_pgm_fn,
	fpga_init_fn,
	NULL,
	fpga_done_fn,
	fpga_clk_fn,
	fpga_cs_fn,
	fpga_wr_fn,
	fpga_rdata_fn,
	fpga_wdata_fn,
	fpga_busy_fn,
	fpga_abort_fn,
	fpga_post_fn,
};

xilinx_desc fpga[CONFIG_FPGA_COUNT] = {
	{xilinx_spartan3,
	 slave_parallel,
	 1196128l/8,
	 (void *)&fpga_fns,
	 0,
	 &spartan3_op,
	 "3s200aft256"}
};

/*
 * Initialize GPIO port B before download
 */
int fpga_pre_fn(int cookie)
{
	/* Initialize GPIO pins */
	gpio_set_value(ACFG_FPGA_PWR, 1);
	imx_gpio_mode(ACFG_FPGA_INIT | GPIO_IN | GPIO_PUEN | GPIO_GPIO);
	imx_gpio_mode(ACFG_FPGA_DONE | GPIO_IN | GPIO_PUEN | GPIO_GPIO);
	imx_gpio_mode(ACFG_FPGA_PRG | GPIO_OUT | GPIO_PUEN | GPIO_GPIO);
	imx_gpio_mode(ACFG_FPGA_CLK | GPIO_OUT | GPIO_PUEN | GPIO_GPIO);
	imx_gpio_mode(ACFG_FPGA_RW | GPIO_OUT | GPIO_PUEN | GPIO_GPIO);
	imx_gpio_mode(ACFG_FPGA_CS | GPIO_OUT | GPIO_PUEN | GPIO_GPIO);
	imx_gpio_mode(ACFG_FPGA_SUSPEND|GPIO_OUT|GPIO_PUEN|GPIO_GPIO);
	gpio_set_value(ACFG_FPGA_RESET, 1);
	imx_gpio_mode(ACFG_FPGA_RESET | GPIO_OUT | GPIO_PUEN | GPIO_GPIO);
	imx_gpio_mode(ACFG_FPGA_PWR | GPIO_OUT | GPIO_PUEN | GPIO_GPIO);
	gpio_set_value(ACFG_FPGA_PRG, 1);
	gpio_set_value(ACFG_FPGA_CLK, 1);
	gpio_set_value(ACFG_FPGA_RW, 1);
	gpio_set_value(ACFG_FPGA_CS, 1);
	gpio_set_value(ACFG_FPGA_SUSPEND, 0);
	gpio_set_value(ACFG_FPGA_PWR, 0);
	udelay(30000); /*wait until supply started*/

	return cookie;
}

/*
 * Set the FPGA's active-low program line to the specified level
 */
int fpga_pgm_fn(int assert, int flush, int cookie)
{
	debug("%s:%d: FPGA PROGRAM %s", __func__, __LINE__,
	      assert ? "high" : "low");
	gpio_set_value(ACFG_FPGA_PRG, !assert);
	return assert;
}

/*
 * Set the FPGA's active-high clock line to the specified level
 */
int fpga_clk_fn(int assert_clk, int flush, int cookie)
{
	debug("%s:%d: FPGA CLOCK %s", __func__, __LINE__,
	      assert_clk ? "high" : "low");
	gpio_set_value(ACFG_FPGA_CLK, !assert_clk);
	return assert_clk;
}

/*
 * Test the state of the active-low FPGA INIT line.  Return 1 on INIT
 * asserted (low).
 */
int fpga_init_fn(int cookie)
{
	int value;
	debug("%s:%d: INIT check... ", __func__, __LINE__);
	value = gpio_get_value(ACFG_FPGA_INIT);
	/* printf("init value read %x",value); */
#ifdef CONFIG_SYS_FPGA_IS_PROTO
	return value;
#else
	return !value;
#endif
}

/*
 * Test the state of the active-high FPGA DONE pin
 */
int fpga_done_fn(int cookie)
{
	debug("%s:%d: DONE check... %s", __func__, __LINE__,
	      gpio_get_value(ACFG_FPGA_DONE) ? "high" : "low");
	return gpio_get_value(ACFG_FPGA_DONE) ? FPGA_SUCCESS : FPGA_FAIL;
}

/*
 * Set the FPGA's wr line to the specified level
 */
int fpga_wr_fn(int assert_write, int flush, int cookie)
{
	debug("%s:%d: FPGA RW... %s ", __func__, __LINE__,
	      assert_write ? "high" : "low");
	gpio_set_value(ACFG_FPGA_RW, !assert_write);
	return assert_write;
}

int fpga_cs_fn(int assert_cs, int flush, int cookie)
{
	debug("%s:%d: FPGA CS %s ", __func__, __LINE__,
	      assert_cs ? "high" : "low");
	gpio_set_value(ACFG_FPGA_CS, !assert_cs);
	return assert_cs;
}

int fpga_rdata_fn(unsigned char *data, int cookie)
{
	debug("%s:%d: FPGA READ DATA %02X ", __func__, __LINE__,
	      *((char *)ACFG_FPGA_RDATA));
	*data = (unsigned char)
		((*((unsigned short *)ACFG_FPGA_RDATA))&0x00FF);
	return *data;
}

int fpga_wdata_fn(unsigned char data, int flush, int cookie)
{
	debug("%s:%d: FPGA WRITE DATA %02X ", __func__, __LINE__,
	      data);
	*((unsigned short *)ACFG_FPGA_WDATA) = data;
	return data;
}

int fpga_abort_fn(int cookie)
{
	return fpga_post_fn(cookie);
}


int fpga_busy_fn(int cookie)
{
	return 1;
}

int fpga_post_fn(int cookie)
{
	debug("%s:%d: FPGA POST ", __func__, __LINE__);

	imx_gpio_mode(ACFG_FPGA_RW | GPIO_PF | GPIO_PUEN);
	imx_gpio_mode(ACFG_FPGA_CS | GPIO_PF | GPIO_PUEN);
	imx_gpio_mode(ACFG_FPGA_CLK | GPIO_PF | GPIO_PUEN);
	gpio_set_value(ACFG_FPGA_PRG, 1);
	gpio_set_value(ACFG_FPGA_RESET, 0);
	imx_gpio_mode(ACFG_FPGA_RESET | GPIO_OUT | GPIO_PUEN | GPIO_GPIO);
	return cookie;
}

void apf27_fpga_setup(void)
{
	struct pll_regs *pll = (struct pll_regs *)IMX_PLL_BASE;
	struct system_control_regs *system =
		(struct system_control_regs *)IMX_SYSTEM_CTL_BASE;

	/* Configure FPGA CLKO */
	writel(ACFG_CCSR_VAL, &pll->ccsr);

	/* Configure strentgh for FPGA */
	writel(ACFG_DSCR10_VAL, &system->dscr10);
	writel(ACFG_DSCR3_VAL, &system->dscr3);
	writel(ACFG_DSCR7_VAL, &system->dscr7);
	writel(ACFG_DSCR2_VAL, &system->dscr2);
}

/*
 * Initialize the fpga.  Return 1 on success, 0 on failure.
 */
void APF27_init_fpga(void)
{
	int i;

	apf27_fpga_setup();

	fpga_init();

	for (i = 0; i < CONFIG_FPGA_COUNT; i++) {
		debug("%s:%d: Adding fpga %d\n", __func__, __LINE__, i);
		fpga_add(fpga_xilinx, &fpga[i]);
	}

	return;
}
