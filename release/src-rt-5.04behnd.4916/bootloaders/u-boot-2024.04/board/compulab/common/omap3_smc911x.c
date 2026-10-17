// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014 CompuLab, Ltd. <www.compulab.co.il>
 *
 * Authors: Igor Grinberg <grinberg@compulab.co.il>
 */

#include <common.h>
#include <netdev.h>
#include <linux/delay.h>

#include <asm/io.h>
#include <linux/errno.h>
#include <asm/arch/cpu.h>
#include <asm/arch/mem.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>

#include "common.h"

static u32 cl_omap3_smc911x_gpmc_net_config[GPMC_MAX_REG] = {
	NET_GPMC_CONFIG1,
	NET_GPMC_CONFIG2,
	NET_GPMC_CONFIG3,
	NET_GPMC_CONFIG4,
	NET_GPMC_CONFIG5,
	NET_GPMC_CONFIG6,
	0
};

static void cl_omap3_smc911x_setup_net_chip_gmpc(int cs, u32 base_addr)
{
	struct ctrl *ctrl_base = (struct ctrl *)OMAP34XX_CTRL_BASE;

	enable_gpmc_cs_config(cl_omap3_smc911x_gpmc_net_config,
			      &gpmc_cfg->cs[cs], base_addr, GPMC_SIZE_16M);

	/* Enable off mode for NWE in PADCONF_GPMC_NWE register */
	writew(readw(&ctrl_base->gpmc_nwe) | 0x0E00, &ctrl_base->gpmc_nwe);

	/* Enable off mode for NOE in PADCONF_GPMC_NADV_ALE register */
	writew(readw(&ctrl_base->gpmc_noe) | 0x0E00, &ctrl_base->gpmc_noe);

	/* Enable off mode for ALE in PADCONF_GPMC_NADV_ALE register */
	writew(readw(&ctrl_base->gpmc_nadv_ale) | 0x0E00,
	       &ctrl_base->gpmc_nadv_ale);
}

#ifdef CONFIG_OMAP_GPIO
static int cl_omap3_smc911x_reset_net_chip(int gpio)
{
	int err;

	if (!gpio_is_valid(gpio))
		return -EINVAL;

	err = gpio_request(gpio, "eth rst");
	if (err)
		return err;

	/* Set gpio as output and send a pulse */
	gpio_direction_output(gpio, 1);
	udelay(1);
	gpio_set_value(gpio, 0);
	mdelay(40);
	gpio_set_value(gpio, 1);
	mdelay(1);

	return 0;
}
#else /* !CONFIG_OMAP_GPIO */
static inline int cl_omap3_smc911x_reset_net_chip(int gpio) { return 0; }
#endif /* CONFIG_OMAP_GPIO */

int cl_omap3_smc911x_init(int id, int cs, u32 base_addr,
			  int (*reset)(int), int rst_gpio)
{
	int ret;

	cl_omap3_smc911x_setup_net_chip_gmpc(cs, base_addr);

	if (reset)
		reset(rst_gpio);
	else
		cl_omap3_smc911x_reset_net_chip(rst_gpio);

	ret = smc911x_initialize(id, base_addr);
	if (ret > 0)
		return ret;

	printf("Failed initializing SMC911x! ");
	return 0;
}
