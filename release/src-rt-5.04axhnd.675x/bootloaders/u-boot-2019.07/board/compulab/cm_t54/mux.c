// SPDX-License-Identifier: GPL-2.0+
/*
 * Pinmux configuration for Compulab CM-T54 board
 *
 * Copyright (C) 2014, Compulab Ltd - http://compulab.co.il/
 *
 * Author: Dmitry Lifshitz <lifshitz@compulab.co.il>
 */

#ifndef _CM_T54_MUX_DATA_H
#define _CM_T54_MUX_DATA_H

#include <asm/arch/mux_omap5.h>
#include <asm/arch/sys_proto.h>

const struct pad_conf_entry core_padconf_array_essential[] = {
	/* MMC1 - SD CARD */
	{SDCARD_CLK, (PTU | IEN | M0)},			/* SDCARD_CLK */
	{SDCARD_CMD, (PTU | IEN | M0)},			/* SDCARD_CMD */
	{SDCARD_DATA0, (PTU | IEN | M0)},		/* SDCARD_DATA0 */
	{SDCARD_DATA1, (PTU | IEN | M0)},		/* SDCARD_DATA1 */
	{SDCARD_DATA2, (PTU | IEN | M0)},		/* SDCARD_DATA2 */
	{SDCARD_DATA3, (PTU | IEN | M0)},		/* SDCARD_DATA3 */

	/* SD CARD CD and WP GPIOs*/
	{TIMER5_PWM_EVT, (PTU | IEN | M6)},		/* GPIO8_228 */
	{TIMER6_PWM_EVT, (PTU | IEN | M6)},		/* GPIO8_229 */

	/* MMC2 - eMMC */
	{EMMC_CLK, (PTU | IEN | M0)},			/* EMMC_CLK */
	{EMMC_CMD, (PTU | IEN | M0)},			/* EMMC_CMD */
	{EMMC_DATA0, (PTU | IEN | M0)},			/* EMMC_DATA0 */
	{EMMC_DATA1, (PTU | IEN | M0)},			/* EMMC_DATA1 */
	{EMMC_DATA2, (PTU | IEN | M0)},			/* EMMC_DATA2 */
	{EMMC_DATA3, (PTU | IEN | M0)},			/* EMMC_DATA3 */
	{EMMC_DATA4, (PTU | IEN | M0)},			/* EMMC_DATA4 */
	{EMMC_DATA5, (PTU | IEN | M0)},			/* EMMC_DATA5 */
	{EMMC_DATA6, (PTU | IEN | M0)},			/* EMMC_DATA6 */
	{EMMC_DATA7, (PTU | IEN | M0)},			/* EMMC_DATA7 */

	/* UART4 */
	{I2C5_SCL, (PTU | IEN | M2)},			/* UART4_RX */
	{I2C5_SDA, (M2)},				/* UART4_TX */

	/* Led */
	{HSI2_CAFLAG, (PTU | M6)},			/* GPIO3_80 */

	/* I2C1 */
	{I2C1_PMIC_SCL, (PTU | IEN | M0)},		/* I2C1_PMIC_SCL */
	{I2C1_PMIC_SDA, (PTU | IEN | M0)},		/* I2C1_PMIC_SDA */

	/* USBB2, USBB3 */
	{USBB2_HSIC_STROBE, (PTU | IEN | M0)},		/* USBB2_HSIC_STROBE */
	{USBB2_HSIC_DATA, (PTU | IEN | M0)},		/* USBB2_HSIC_DATA */
	{USBB3_HSIC_STROBE, (PTU | IEN | M0)},		/* USBB3_HSIC_STROBE */
	{USBB3_HSIC_DATA, (PTU | IEN | M0)},		/* USBB3_HSIC_DATA */

	/* USB Hub and USB Eth reset GPIOs */
	{HSI2_CAREADY, (PTD | M6)},			/* GPIO3_76 */
	{HSI2_ACDATA, (PTD | M6)},			/* GPIO3_83 */

	/* I2C4 */
	{I2C4_SCL, (PTU | IEN | M0)},			/* I2C4_SCL  */
	{I2C4_SDA, (PTU | IEN | M0)},			/* I2C4_SDA  */
};

const struct pad_conf_entry wkup_padconf_array_essential[] = {
	{SR_PMIC_SCL, (PTU | IEN | M0)},		/* SR_PMIC_SCL */
	{SR_PMIC_SDA, (PTU | IEN | M0)},		/* SR_PMIC_SDA */
	{SYS_32K, (IEN | M0)},				/* SYS_32K */

	/* USB Hub clock */
	{FREF_CLK1_OUT, (PTD | IEN | M0)},		/* FREF_CLK1_OUT  */
};

/*
 * Routine: set_muxconf_regs
 * Description: setup board pinmux configuration.
 */
void set_muxconf_regs(void)
{
	do_set_mux((*ctrl)->control_padconf_core_base,
		   core_padconf_array_essential,
		   sizeof(core_padconf_array_essential) /
		   sizeof(struct pad_conf_entry));

	do_set_mux((*ctrl)->control_padconf_wkup_base,
		   wkup_padconf_array_essential,
		   sizeof(wkup_padconf_array_essential) /
		   sizeof(struct pad_conf_entry));
}

#endif /* _CM_T54_MUX_DATA_H */
