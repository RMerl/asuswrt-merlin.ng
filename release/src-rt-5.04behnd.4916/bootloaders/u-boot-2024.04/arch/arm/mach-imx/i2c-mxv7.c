// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2012 Boundary Devices Inc.
 */
#include <common.h>
#include <malloc.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <asm/gpio.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <watchdog.h>

int force_idle_bus(void *priv)
{
	int i;
	int sda, scl;
	ulong elapsed, start_time;
	struct i2c_pads_info *p = (struct i2c_pads_info *)priv;
	int ret = 0;

	gpio_direction_input(p->sda.gp);
	gpio_direction_input(p->scl.gp);

	imx_iomux_v3_setup_pad(p->sda.gpio_mode);
	imx_iomux_v3_setup_pad(p->scl.gpio_mode);

	sda = gpio_get_value(p->sda.gp);
	scl = gpio_get_value(p->scl.gp);
	if ((sda & scl) == 1)
		goto exit;		/* Bus is idle already */

	printf("%s: sda=%d scl=%d sda.gp=0x%x scl.gp=0x%x\n", __func__,
		sda, scl, p->sda.gp, p->scl.gp);
	/* Send high and low on the SCL line */
	for (i = 0; i < 9; i++) {
		gpio_direction_output(p->scl.gp, 0);
		udelay(50);
		gpio_direction_input(p->scl.gp);
		udelay(50);
	}
	start_time = get_timer(0);
	for (;;) {
		sda = gpio_get_value(p->sda.gp);
		scl = gpio_get_value(p->scl.gp);
		if ((sda & scl) == 1)
			break;
		schedule();
		elapsed = get_timer(start_time);
		if (elapsed > (CONFIG_SYS_HZ / 5)) {	/* .2 seconds */
			ret = -EBUSY;
			printf("%s: failed to clear bus, sda=%d scl=%d\n",
					__func__, sda, scl);
			break;
		}
	}
exit:
	imx_iomux_v3_setup_pad(p->sda.i2c_mode);
	imx_iomux_v3_setup_pad(p->scl.i2c_mode);
	return ret;
}

static void * const i2c_bases[] = {
	(void *)I2C1_BASE_ADDR,
	(void *)I2C2_BASE_ADDR,
#ifdef I2C3_BASE_ADDR
	(void *)I2C3_BASE_ADDR,
#endif
#ifdef I2C4_BASE_ADDR
	(void *)I2C4_BASE_ADDR,
#endif
#ifdef I2C5_BASE_ADDR
	(void *)I2C5_BASE_ADDR,
#endif
#ifdef I2C6_BASE_ADDR
	(void *)I2C6_BASE_ADDR,
#endif
};

/* i2c_index can be from 0 - 3 */
int setup_i2c(unsigned i2c_index, int speed, int slave_addr,
	      struct i2c_pads_info *p)
{
	char name[9];
	int ret;

	if (i2c_index >= ARRAY_SIZE(i2c_bases))
		return -EINVAL;

	snprintf(name, sizeof(name), "i2c_sda%01d", i2c_index);
	ret = gpio_request(p->sda.gp, name);
	if (ret)
		return ret;

	snprintf(name, sizeof(name), "i2c_scl%01d", i2c_index);
	ret = gpio_request(p->scl.gp, name);
	if (ret)
		goto err_req;

	/* Enable i2c clock */
	ret = enable_i2c_clk(1, i2c_index);
	if (ret)
		goto err_clk;

	/* Make sure bus is idle */
	ret = force_idle_bus(p);
	if (ret)
		goto err_idle;

#if !CONFIG_IS_ENABLED(DM_I2C)
	bus_i2c_init(i2c_index, speed, slave_addr, force_idle_bus, p);
#endif

	return 0;

err_idle:
err_clk:
	gpio_free(p->scl.gp);
err_req:
	gpio_free(p->sda.gp);

	return ret;
}
