// SPDX-License-Identifier: GPL-2.0+
/*
 *
 * Common functions for OMAP4 based boards
 *
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 *
 * Author :
 *	Aneesh V	<aneesh@ti.com>
 *	Steve Sakoman	<steve@sakoman.com>
 */
#include <common.h>
#include <palmas.h>
#include <asm/armv7.h>
#include <asm/arch/cpu.h>
#include <asm/arch/sys_proto.h>
#include <linux/sizes.h>
#include <asm/emif.h>
#include <asm/arch/gpio.h>
#include <asm/omap_common.h>

u32 *const omap_si_rev = (u32 *)OMAP_SRAM_SCRATCH_OMAP_REV;

static const struct gpio_bank gpio_bank_44xx[6] = {
	{ (void *)OMAP44XX_GPIO1_BASE },
	{ (void *)OMAP44XX_GPIO2_BASE },
	{ (void *)OMAP44XX_GPIO3_BASE },
	{ (void *)OMAP44XX_GPIO4_BASE },
	{ (void *)OMAP44XX_GPIO5_BASE },
	{ (void *)OMAP44XX_GPIO6_BASE },
};

const struct gpio_bank *const omap_gpio_bank = gpio_bank_44xx;

#ifdef CONFIG_SPL_BUILD
/*
 * Some tuning of IOs for optimal power and performance
 */
void do_io_settings(void)
{
	u32 lpddr2io;

	u32 omap4_rev = omap_revision();

	if (omap4_rev == OMAP4430_ES1_0)
		lpddr2io = CONTROL_LPDDR2IO_SLEW_125PS_DRV8_PULL_DOWN;
	else if (omap4_rev == OMAP4430_ES2_0)
		lpddr2io = CONTROL_LPDDR2IO_SLEW_325PS_DRV8_GATE_KEEPER;
	else
		lpddr2io = CONTROL_LPDDR2IO_SLEW_315PS_DRV12_PULL_DOWN;

	/* EMIF1 */
	writel(lpddr2io, (*ctrl)->control_lpddr2io1_0);
	writel(lpddr2io, (*ctrl)->control_lpddr2io1_1);
	/* No pull for GR10 as per hw team's recommendation */
	writel(lpddr2io & ~LPDDR2IO_GR10_WD_MASK,
		(*ctrl)->control_lpddr2io1_2);
	writel(CONTROL_LPDDR2IO_3_VAL, (*ctrl)->control_lpddr2io1_3);

	/* EMIF2 */
	writel(lpddr2io, (*ctrl)->control_lpddr2io2_0);
	writel(lpddr2io, (*ctrl)->control_lpddr2io2_1);
	/* No pull for GR10 as per hw team's recommendation */
	writel(lpddr2io & ~LPDDR2IO_GR10_WD_MASK,
		(*ctrl)->control_lpddr2io2_2);
	writel(CONTROL_LPDDR2IO_3_VAL, (*ctrl)->control_lpddr2io2_3);

	/*
	 * Some of these settings (TRIM values) come from eFuse and are
	 * in turn programmed in the eFuse at manufacturing time after
	 * calibration of the device. Do the software over-ride only if
	 * the device is not correctly trimmed
	 */
	if (!(readl((*ctrl)->control_std_fuse_opp_bgap) & 0xFFFF)) {

		writel(LDOSRAM_VOLT_CTRL_OVERRIDE,
			(*ctrl)->control_ldosram_iva_voltage_ctrl);

		writel(LDOSRAM_VOLT_CTRL_OVERRIDE,
			(*ctrl)->control_ldosram_mpu_voltage_ctrl);

		writel(LDOSRAM_VOLT_CTRL_OVERRIDE,
			(*ctrl)->control_ldosram_core_voltage_ctrl);
	}

	/*
	 * Over-ride the register
	 *	i. unconditionally for all 4430
	 *	ii. only if un-trimmed for 4460
	 */
	if (!readl((*ctrl)->control_efuse_1))
		writel(CONTROL_EFUSE_1_OVERRIDE, (*ctrl)->control_efuse_1);

	if ((omap4_rev < OMAP4460_ES1_0) || !readl((*ctrl)->control_efuse_2))
		writel(CONTROL_EFUSE_2_OVERRIDE, (*ctrl)->control_efuse_2);
}
#endif /* CONFIG_SPL_BUILD */

/* dummy fuction for omap4 */
void config_data_eye_leveling_samples(u32 emif_base)
{
}

void init_omap_revision(void)
{
	/*
	 * For some of the ES2/ES1 boards ID_CODE is not reliable:
	 * Also, ES1 and ES2 have different ARM revisions
	 * So use ARM revision for identification
	 */
	unsigned int arm_rev = cortex_rev();

	switch (arm_rev) {
	case MIDR_CORTEX_A9_R0P1:
		*omap_si_rev = OMAP4430_ES1_0;
		break;
	case MIDR_CORTEX_A9_R1P2:
		switch (readl(CONTROL_ID_CODE)) {
		case OMAP4_CONTROL_ID_CODE_ES2_0:
			*omap_si_rev = OMAP4430_ES2_0;
			break;
		case OMAP4_CONTROL_ID_CODE_ES2_1:
			*omap_si_rev = OMAP4430_ES2_1;
			break;
		case OMAP4_CONTROL_ID_CODE_ES2_2:
			*omap_si_rev = OMAP4430_ES2_2;
			break;
		default:
			*omap_si_rev = OMAP4430_ES2_0;
			break;
		}
		break;
	case MIDR_CORTEX_A9_R1P3:
		*omap_si_rev = OMAP4430_ES2_3;
		break;
	case MIDR_CORTEX_A9_R2P10:
		switch (readl(CONTROL_ID_CODE)) {
		case OMAP4470_CONTROL_ID_CODE_ES1_0:
			*omap_si_rev = OMAP4470_ES1_0;
			break;
		case OMAP4460_CONTROL_ID_CODE_ES1_1:
			*omap_si_rev = OMAP4460_ES1_1;
			break;
		case OMAP4460_CONTROL_ID_CODE_ES1_0:
		default:
			*omap_si_rev = OMAP4460_ES1_0;
			break;
		}
		break;
	default:
		*omap_si_rev = OMAP4430_SILICON_ID_INVALID;
		break;
	}
}

void omap_die_id(unsigned int *die_id)
{
	die_id[0] = readl((*ctrl)->control_std_fuse_die_id_0);
	die_id[1] = readl((*ctrl)->control_std_fuse_die_id_1);
	die_id[2] = readl((*ctrl)->control_std_fuse_die_id_2);
	die_id[3] = readl((*ctrl)->control_std_fuse_die_id_3);
}

#ifndef CONFIG_SYS_L2CACHE_OFF
void v7_outer_cache_enable(void)
{
	omap_smc1(OMAP4_SERVICE_PL310_CONTROL_REG_SET, 1);
}

void v7_outer_cache_disable(void)
{
	omap_smc1(OMAP4_SERVICE_PL310_CONTROL_REG_SET, 0);
}
#endif /* !CONFIG_SYS_L2CACHE_OFF */

void vmmc_pbias_config(uint voltage)
{
	u32 value = 0;

	value = readl((*ctrl)->control_pbiaslite);
	value &= ~(MMC1_PBIASLITE_PWRDNZ | MMC1_PWRDNZ);
	writel(value, (*ctrl)->control_pbiaslite);
	value = readl((*ctrl)->control_pbiaslite);
	value |= MMC1_PBIASLITE_VMODE | MMC1_PBIASLITE_PWRDNZ | MMC1_PWRDNZ;
	writel(value, (*ctrl)->control_pbiaslite);
}
