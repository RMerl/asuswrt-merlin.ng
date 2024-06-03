// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2007-2012
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Tom Cubie <tangliang@allwinnertech.com>
 *
 * (C) Copyright 2013 Luke Kenneth Casson Leighton <lkcl@lkcl.net>
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/gpio.h>
#include <asm/arch/prcm.h>
#include <asm/arch/gtbus.h>
#include <asm/arch/sys_proto.h>

__weak void clock_init_sec(void)
{
}

__weak void gtbus_init(void)
{
}

int clock_init(void)
{
#ifdef CONFIG_SPL_BUILD
	clock_init_safe();
	gtbus_init();
#endif
	clock_init_uart();
	clock_init_sec();

	return 0;
}

/* These functions are shared between various SoCs so put them here. */
#if defined CONFIG_SUNXI_GEN_SUN6I && !defined CONFIG_MACH_SUN9I
int clock_twi_onoff(int port, int state)
{
	struct sunxi_ccm_reg *const ccm =
		(struct sunxi_ccm_reg *)SUNXI_CCM_BASE;

	if (port == 5) {
		if (state)
			prcm_apb0_enable(
				PRCM_APB0_GATE_PIO | PRCM_APB0_GATE_I2C);
		else
			prcm_apb0_disable(
				PRCM_APB0_GATE_PIO | PRCM_APB0_GATE_I2C);
		return 0;
	}

	/* set the apb clock gate and reset for twi */
	if (state) {
		setbits_le32(&ccm->apb2_gate,
			     CLK_GATE_OPEN << (APB2_GATE_TWI_SHIFT + port));
		setbits_le32(&ccm->apb2_reset_cfg,
			     1 << (APB2_RESET_TWI_SHIFT + port));
	} else {
		clrbits_le32(&ccm->apb2_reset_cfg,
			     1 << (APB2_RESET_TWI_SHIFT + port));
		clrbits_le32(&ccm->apb2_gate,
			     CLK_GATE_OPEN << (APB2_GATE_TWI_SHIFT + port));
	}

	return 0;
}
#endif
