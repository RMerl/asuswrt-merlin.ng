// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Stefan Roese <sr@denx.de>
 */

#include <common.h>
#include <altera.h>
#include <errno.h>
#include <log.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>
#include <asm/arch-mvebu/spi.h>
#include "theadorable.h"

/*
 * FPGA programming support
 */
static int fpga_pre_fn(int cookie)
{
	int gpio_config = COOKIE2CONFIG(cookie);
	int gpio_done = COOKIE2DONE(cookie);
	int ret;

	debug("%s (%d): cookie=%08x gpio_config=%d gpio_done=%d\n",
	      __func__, __LINE__, cookie, gpio_config, gpio_done);

	/* Configure config pin */
	/* Set to output */
	ret = gpio_request(gpio_config, "CONFIG");
	if (ret < 0)
		return ret;
	gpio_direction_output(gpio_config, 1);

	/* Configure done pin */
	/* Set to input */
	ret = gpio_request(gpio_done, "DONE");
	if (ret < 0)
		return ret;

	gpio_direction_input(gpio_done);

	return 0;
}

static int fpga_config_fn(int assert, int flush, int cookie)
{
	int gpio_config = COOKIE2CONFIG(cookie);

	debug("%s (%d): cookie=%08x gpio_config=%d\n",
	      __func__, __LINE__, cookie, gpio_config);

	if (assert)
		gpio_set_value(gpio_config, 1);
	else
		gpio_set_value(gpio_config, 0);

	return 0;
}

static int fpga_write_fn(const void *buf, size_t len, int flush, int cookie)
{
	int spi_bus = COOKIE2SPI_BUS(cookie);
	int spi_dev = COOKIE2SPI_DEV(cookie);
	struct kwspi_registers *reg;
	u32 control_reg;
	u32 config_reg;
	void *dst;

	/*
	 * Write data to FPGA attached to SPI bus via SPI direct write.
	 * This results in the fastest and easiest way to program the
	 * bitstream into the FPGA.
	 */
	debug("%s (%d): cookie=%08x spi_bus=%d spi_dev=%d\n",
	      __func__, __LINE__, cookie, spi_bus, spi_dev);

	if (spi_bus == 0) {
		reg = (struct kwspi_registers *)MVEBU_REGISTER(0x10600);
		dst = (void *)SPI_BUS0_DEV1_BASE;
	} else {
		reg = (struct kwspi_registers *)MVEBU_REGISTER(0x10680);
		dst = (void *)SPI_BUS1_DEV2_BASE;
	}

	/* Configure SPI controller for direct access mode */
	control_reg = readl(&reg->ctrl);
	config_reg = readl(&reg->cfg);
	writel(0x00000214, &reg->cfg);		/* 27MHz clock */
	writel(0x00000000, &reg->dw_cfg);	/* don't de-asset CS */
	writel(KWSPI_CSN_ACT, &reg->ctrl);	/* activate CS */

	/* Copy data to the SPI direct mapped window */
	memcpy(dst, buf, len);

	/* Restore original register values */
	writel(control_reg, &reg->ctrl);
	writel(config_reg, &reg->cfg);

	return 0;
}

/* Returns the state of CONF_DONE Pin */
static int fpga_done_fn(int cookie)
{
	int gpio_done = COOKIE2DONE(cookie);
	unsigned long ts;

	debug("%s (%d): cookie=%08x gpio_done=%d\n",
	      __func__, __LINE__, cookie, gpio_done);

	ts = get_timer(0);
	do {
		if (gpio_get_value(gpio_done))
			return 0;
	} while (get_timer(ts) < 1000);

	/* timeout so return error */
	return -ENODEV;
}

static altera_board_specific_func stratixv_fns = {
	.pre = fpga_pre_fn,
	.config = fpga_config_fn,
	.write = fpga_write_fn,
	.done = fpga_done_fn,
};

static Altera_desc altera_fpga[] = {
	{
		/* Family */
		Altera_StratixV,
		/* Interface type */
		passive_serial,
		/* No limitation as additional data will be ignored */
		-1,
		/* Device function table */
		(void *)&stratixv_fns,
		/* Base interface address specified in driver */
		NULL,
		/* Cookie implementation */
		/*
		 * In this 32bit word the following information is coded:
		 * Bit 31 ... Bit 0
		 * SPI-Bus | SPI-Dev | Config-Pin | Done-Pin
		 */
		FPGA_COOKIE(0, 1, 26, 7)
	},
	{
		/* Family */
		Altera_StratixV,
		/* Interface type */
		passive_serial,
		/* No limitation as additional data will be ignored */
		-1,
		/* Device function table */
		(void *)&stratixv_fns,
		/* Base interface address specified in driver */
		NULL,
		/* Cookie implementation */
		/*
		 * In this 32bit word the following information is coded:
		 * Bit 31 ... Bit 0
		 * SPI-Bus | SPI-Dev | Config-Pin | Done-Pin
		 */
		FPGA_COOKIE(1, 2, 29, 9)
	},
};

/* Add device descriptor to FPGA device table */
void board_fpga_add(void)
{
	int i;

	fpga_init();
	for (i = 0; i < ARRAY_SIZE(altera_fpga); i++)
		fpga_add(fpga_altera, &altera_fpga[i]);
}
