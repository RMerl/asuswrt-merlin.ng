// SPDX-License-Identifier: GPL-2.0+
/*
 *
 * Functions for omap5 based boards.
 *
 * (C) Copyright 2011
 * Texas Instruments, <www.ti.com>
 *
 * Author :
 *	Aneesh V	<aneesh@ti.com>
 *	Steve Sakoman	<steve@sakoman.com>
 *	Sricharan	<r.sricharan@ti.com>
 */
#include <common.h>
#include <cpu_func.h>
#include <palmas.h>
#include <asm/armv7.h>
#include <asm/arch/cpu.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/clock.h>
#include <linux/delay.h>
#include <linux/sizes.h>
#include <asm/utils.h>
#include <asm/arch/gpio.h>
#include <asm/emif.h>
#include <asm/omap_common.h>

u32 *const omap_si_rev = (u32 *)OMAP_SRAM_SCRATCH_OMAP_REV;

#if !CONFIG_IS_ENABLED(DM_GPIO)
static struct gpio_bank gpio_bank_54xx[8] = {
	{ (void *)OMAP54XX_GPIO1_BASE },
	{ (void *)OMAP54XX_GPIO2_BASE },
	{ (void *)OMAP54XX_GPIO3_BASE },
	{ (void *)OMAP54XX_GPIO4_BASE },
	{ (void *)OMAP54XX_GPIO5_BASE },
	{ (void *)OMAP54XX_GPIO6_BASE },
	{ (void *)OMAP54XX_GPIO7_BASE },
	{ (void *)OMAP54XX_GPIO8_BASE },
};

const struct gpio_bank *const omap_gpio_bank = gpio_bank_54xx;
#endif

void do_set_mux32(u32 base, struct pad_conf_entry const *array, int size)
{
	int i;
	struct pad_conf_entry *pad = (struct pad_conf_entry *)array;

	for (i = 0; i < size; i++, pad++)
		writel(pad->val, base + pad->offset);
}

#ifdef CONFIG_SPL_BUILD
/* LPDDR2 specific IO settings */
static void io_settings_lpddr2(void)
{
	const struct ctrl_ioregs *ioregs;

	get_ioregs(&ioregs);
	writel(ioregs->ctrl_ddrch, (*ctrl)->control_ddrch1_0);
	writel(ioregs->ctrl_ddrch, (*ctrl)->control_ddrch1_1);
	writel(ioregs->ctrl_ddrch, (*ctrl)->control_ddrch2_0);
	writel(ioregs->ctrl_ddrch, (*ctrl)->control_ddrch2_1);
	writel(ioregs->ctrl_lpddr2ch, (*ctrl)->control_lpddr2ch1_0);
	writel(ioregs->ctrl_lpddr2ch, (*ctrl)->control_lpddr2ch1_1);
	writel(ioregs->ctrl_ddrio_0, (*ctrl)->control_ddrio_0);
	writel(ioregs->ctrl_ddrio_1, (*ctrl)->control_ddrio_1);
	writel(ioregs->ctrl_ddrio_2, (*ctrl)->control_ddrio_2);
}

/* DDR3 specific IO settings */
static void io_settings_ddr3(void)
{
	u32 io_settings = 0;
	const struct ctrl_ioregs *ioregs;

	get_ioregs(&ioregs);
	writel(ioregs->ctrl_ddr3ch, (*ctrl)->control_ddr3ch1_0);
	writel(ioregs->ctrl_ddrch, (*ctrl)->control_ddrch1_0);
	writel(ioregs->ctrl_ddrch, (*ctrl)->control_ddrch1_1);

	writel(ioregs->ctrl_ddr3ch, (*ctrl)->control_ddr3ch2_0);
	writel(ioregs->ctrl_ddrch, (*ctrl)->control_ddrch2_0);
	writel(ioregs->ctrl_ddrch, (*ctrl)->control_ddrch2_1);

	writel(ioregs->ctrl_ddrio_0, (*ctrl)->control_ddrio_0);
	writel(ioregs->ctrl_ddrio_1, (*ctrl)->control_ddrio_1);

	if (!is_dra7xx()) {
		writel(ioregs->ctrl_ddrio_2, (*ctrl)->control_ddrio_2);
		writel(ioregs->ctrl_lpddr2ch, (*ctrl)->control_lpddr2ch1_1);
	}

	/* omap5432 does not use lpddr2 */
	writel(ioregs->ctrl_lpddr2ch, (*ctrl)->control_lpddr2ch1_0);

	writel(ioregs->ctrl_emif_sdram_config_ext,
	       (*ctrl)->control_emif1_sdram_config_ext);
	if (!is_dra72x())
		writel(ioregs->ctrl_emif_sdram_config_ext,
		       (*ctrl)->control_emif2_sdram_config_ext);

	if (is_omap54xx()) {
		/* Disable DLL select */
		io_settings = (readl((*ctrl)->control_port_emif1_sdram_config)
							& 0xFFEFFFFF);
		writel(io_settings,
			(*ctrl)->control_port_emif1_sdram_config);

		io_settings = (readl((*ctrl)->control_port_emif2_sdram_config)
							& 0xFFEFFFFF);
		writel(io_settings,
			(*ctrl)->control_port_emif2_sdram_config);
	} else {
		writel(ioregs->ctrl_ddr_ctrl_ext_0,
				(*ctrl)->control_ddr_control_ext_0);
	}
}

/*
 * Some tuning of IOs for optimal power and performance
 */
void do_io_settings(void)
{
	u32 io_settings = 0, mask = 0;
	struct emif_reg_struct *emif = (struct emif_reg_struct *)EMIF1_BASE;

	/* Impedance settings EMMC, C2C 1,2, hsi2 */
	mask = (ds_mask << 2) | (ds_mask << 8) |
		(ds_mask << 16) | (ds_mask << 18);
	io_settings = readl((*ctrl)->control_smart1io_padconf_0) &
				(~mask);
	io_settings |= (ds_60_ohm << 8) | (ds_45_ohm << 16) |
			(ds_45_ohm << 18) | (ds_60_ohm << 2);
	writel(io_settings, (*ctrl)->control_smart1io_padconf_0);

	/* Impedance settings Mcspi2 */
	mask = (ds_mask << 30);
	io_settings = readl((*ctrl)->control_smart1io_padconf_1) &
			(~mask);
	io_settings |= (ds_60_ohm << 30);
	writel(io_settings, (*ctrl)->control_smart1io_padconf_1);

	/* Impedance settings C2C 3,4 */
	mask = (ds_mask << 14) | (ds_mask << 16);
	io_settings = readl((*ctrl)->control_smart1io_padconf_2) &
			(~mask);
	io_settings |= (ds_45_ohm << 14) | (ds_45_ohm << 16);
	writel(io_settings, (*ctrl)->control_smart1io_padconf_2);

	/* Slew rate settings EMMC, C2C 1,2 */
	mask = (sc_mask << 8) | (sc_mask << 16) | (sc_mask << 18);
	io_settings = readl((*ctrl)->control_smart2io_padconf_0) &
			(~mask);
	io_settings |= (sc_fast << 8) | (sc_na << 16) | (sc_na << 18);
	writel(io_settings, (*ctrl)->control_smart2io_padconf_0);

	/* Slew rate settings hsi2, Mcspi2 */
	mask = (sc_mask << 24) | (sc_mask << 28);
	io_settings = readl((*ctrl)->control_smart2io_padconf_1) &
			(~mask);
	io_settings |= (sc_fast << 28) | (sc_fast << 24);
	writel(io_settings, (*ctrl)->control_smart2io_padconf_1);

	/* Slew rate settings C2C 3,4 */
	mask = (sc_mask << 16) | (sc_mask << 18);
	io_settings = readl((*ctrl)->control_smart2io_padconf_2) &
			(~mask);
	io_settings |= (sc_na << 16) | (sc_na << 18);
	writel(io_settings, (*ctrl)->control_smart2io_padconf_2);

	/* impedance and slew rate settings for usb */
	mask = (usb_i_mask << 29) | (usb_i_mask << 26) | (usb_i_mask << 23) |
		(usb_i_mask << 20) | (usb_i_mask << 17) | (usb_i_mask << 14);
	io_settings = readl((*ctrl)->control_smart3io_padconf_1) &
			(~mask);
	io_settings |= (ds_60_ohm << 29) | (ds_60_ohm << 26) |
		       (ds_60_ohm << 23) | (sc_fast << 20) |
		       (sc_fast << 17) | (sc_fast << 14);
	writel(io_settings, (*ctrl)->control_smart3io_padconf_1);

	if (emif_sdram_type(emif->emif_sdram_config) == EMIF_SDRAM_TYPE_LPDDR2)
		io_settings_lpddr2();
	else
		io_settings_ddr3();
}

static const struct srcomp_params srcomp_parameters[NUM_SYS_CLKS] = {
	{0x45, 0x1},	/* 12 MHz   */
	{-1, -1},	/* 13 MHz   */
	{0x63, 0x2},	/* 16.8 MHz */
	{0x57, 0x2},	/* 19.2 MHz */
	{0x20, 0x1},	/* 26 MHz   */
	{-1, -1},	/* 27 MHz   */
	{0x41, 0x3}	/* 38.4 MHz */
};

void srcomp_enable(void)
{
	u32 srcomp_value, mul_factor, div_factor, clk_val, i;
	u32 sysclk_ind	= get_sys_clk_index();
	u32 omap_rev	= omap_revision();

	if (!is_omap54xx())
		return;

	mul_factor = srcomp_parameters[sysclk_ind].multiply_factor;
	div_factor = srcomp_parameters[sysclk_ind].divide_factor;

	for (i = 0; i < 4; i++) {
		srcomp_value = readl((*ctrl)->control_srcomp_north_side + i*4);
		srcomp_value &=
			~(MULTIPLY_FACTOR_XS_MASK | DIVIDE_FACTOR_XS_MASK);
		srcomp_value |= (mul_factor << MULTIPLY_FACTOR_XS_SHIFT) |
			(div_factor << DIVIDE_FACTOR_XS_SHIFT);
		writel(srcomp_value, (*ctrl)->control_srcomp_north_side + i*4);
	}

	if ((omap_rev == OMAP5430_ES1_0) || (omap_rev == OMAP5432_ES1_0)) {
		clk_val = readl((*prcm)->cm_coreaon_io_srcomp_clkctrl);
		clk_val |= OPTFCLKEN_SRCOMP_FCLK_MASK;
		writel(clk_val, (*prcm)->cm_coreaon_io_srcomp_clkctrl);

		for (i = 0; i < 4; i++) {
			srcomp_value =
				readl((*ctrl)->control_srcomp_north_side + i*4);
			srcomp_value &= ~PWRDWN_XS_MASK;
			writel(srcomp_value,
			       (*ctrl)->control_srcomp_north_side + i*4);

			while (((readl((*ctrl)->control_srcomp_north_side + i*4)
				& SRCODE_READ_XS_MASK) >>
				SRCODE_READ_XS_SHIFT) == 0)
				;

			srcomp_value =
				readl((*ctrl)->control_srcomp_north_side + i*4);
			srcomp_value &= ~OVERRIDE_XS_MASK;
			writel(srcomp_value,
			       (*ctrl)->control_srcomp_north_side + i*4);
		}
	} else {
		srcomp_value = readl((*ctrl)->control_srcomp_east_side_wkup);
		srcomp_value &= ~(MULTIPLY_FACTOR_XS_MASK |
				  DIVIDE_FACTOR_XS_MASK);
		srcomp_value |= (mul_factor << MULTIPLY_FACTOR_XS_SHIFT) |
				(div_factor << DIVIDE_FACTOR_XS_SHIFT);
		writel(srcomp_value, (*ctrl)->control_srcomp_east_side_wkup);

		for (i = 0; i < 4; i++) {
			srcomp_value =
				readl((*ctrl)->control_srcomp_north_side + i*4);
			srcomp_value |= SRCODE_OVERRIDE_SEL_XS_MASK;
			writel(srcomp_value,
			       (*ctrl)->control_srcomp_north_side + i*4);

			srcomp_value =
				readl((*ctrl)->control_srcomp_north_side + i*4);
			srcomp_value &= ~OVERRIDE_XS_MASK;
			writel(srcomp_value,
			       (*ctrl)->control_srcomp_north_side + i*4);
		}

		srcomp_value =
			readl((*ctrl)->control_srcomp_east_side_wkup);
		srcomp_value |= SRCODE_OVERRIDE_SEL_XS_MASK;
		writel(srcomp_value, (*ctrl)->control_srcomp_east_side_wkup);

		srcomp_value =
			readl((*ctrl)->control_srcomp_east_side_wkup);
		srcomp_value &= ~OVERRIDE_XS_MASK;
		writel(srcomp_value, (*ctrl)->control_srcomp_east_side_wkup);

		clk_val = readl((*prcm)->cm_coreaon_io_srcomp_clkctrl);
		clk_val |= OPTFCLKEN_SRCOMP_FCLK_MASK;
		writel(clk_val, (*prcm)->cm_coreaon_io_srcomp_clkctrl);

		clk_val = readl((*prcm)->cm_wkupaon_io_srcomp_clkctrl);
		clk_val |= OPTFCLKEN_SRCOMP_FCLK_MASK;
		writel(clk_val, (*prcm)->cm_wkupaon_io_srcomp_clkctrl);

		for (i = 0; i < 4; i++) {
			while (((readl((*ctrl)->control_srcomp_north_side + i*4)
				& SRCODE_READ_XS_MASK) >>
				SRCODE_READ_XS_SHIFT) == 0)
				;

			srcomp_value =
				readl((*ctrl)->control_srcomp_north_side + i*4);
			srcomp_value &= ~SRCODE_OVERRIDE_SEL_XS_MASK;
			writel(srcomp_value,
			       (*ctrl)->control_srcomp_north_side + i*4);
		}

		while (((readl((*ctrl)->control_srcomp_east_side_wkup) &
			SRCODE_READ_XS_MASK) >> SRCODE_READ_XS_SHIFT) == 0)
			;

		srcomp_value =
			readl((*ctrl)->control_srcomp_east_side_wkup);
		srcomp_value &= ~SRCODE_OVERRIDE_SEL_XS_MASK;
		writel(srcomp_value, (*ctrl)->control_srcomp_east_side_wkup);
	}
}
#endif

void config_data_eye_leveling_samples(u32 emif_base)
{
	const struct ctrl_ioregs *ioregs;

	get_ioregs(&ioregs);

	/*EMIF_SDRAM_CONFIG_EXT-Read data eye leveling no of samples =4*/
	if (emif_base == EMIF1_BASE)
		writel(ioregs->ctrl_emif_sdram_config_ext_final,
		       (*ctrl)->control_emif1_sdram_config_ext);
	else if (emif_base == EMIF2_BASE)
		writel(ioregs->ctrl_emif_sdram_config_ext_final,
		       (*ctrl)->control_emif2_sdram_config_ext);
}

void init_cpu_configuration(void)
{
	u32 l2actlr;

	asm volatile("mrc p15, 1, %0, c15, c0, 0" : "=r"(l2actlr));
	/*
	 * L2ACTLR: Ensure to enable the following:
	 * 3: Disable clean/evict push to external
	 * 4: Disable WriteUnique and WriteLineUnique transactions from master
	 * 8: Disable DVM/CMO message broadcast
	 */
	l2actlr |= 0x118;
	omap_smc1(OMAP5_SERVICE_L2ACTLR_SET, l2actlr);
}

void init_omap_revision(void)
{
	/*
	 * For some of the ES2/ES1 boards ID_CODE is not reliable:
	 * Also, ES1 and ES2 have different ARM revisions
	 * So use ARM revision for identification
	 */
	unsigned int rev = cortex_rev();

	switch (readl(CONTROL_ID_CODE)) {
	case OMAP5430_CONTROL_ID_CODE_ES1_0:
		*omap_si_rev = OMAP5430_ES1_0;
		if (rev == MIDR_CORTEX_A15_R2P2)
			*omap_si_rev = OMAP5430_ES2_0;
		break;
	case OMAP5432_CONTROL_ID_CODE_ES1_0:
		*omap_si_rev = OMAP5432_ES1_0;
		if (rev == MIDR_CORTEX_A15_R2P2)
			*omap_si_rev = OMAP5432_ES2_0;
		break;
	case OMAP5430_CONTROL_ID_CODE_ES2_0:
		*omap_si_rev = OMAP5430_ES2_0;
		break;
	case OMAP5432_CONTROL_ID_CODE_ES2_0:
		*omap_si_rev = OMAP5432_ES2_0;
		break;
	case DRA762_CONTROL_ID_CODE_ES1_0:
		*omap_si_rev = DRA762_ES1_0;
		break;
	case DRA752_CONTROL_ID_CODE_ES1_0:
		*omap_si_rev = DRA752_ES1_0;
		break;
	case DRA752_CONTROL_ID_CODE_ES1_1:
		*omap_si_rev = DRA752_ES1_1;
		break;
	case DRA752_CONTROL_ID_CODE_ES2_0:
		*omap_si_rev = DRA752_ES2_0;
		break;
	case DRA722_CONTROL_ID_CODE_ES1_0:
		*omap_si_rev = DRA722_ES1_0;
		break;
	case DRA722_CONTROL_ID_CODE_ES2_0:
		*omap_si_rev = DRA722_ES2_0;
		break;
	case DRA722_CONTROL_ID_CODE_ES2_1:
		*omap_si_rev = DRA722_ES2_1;
		break;
	default:
		*omap_si_rev = OMAP5430_SILICON_ID_INVALID;
	}
	init_cpu_configuration();
}

void init_package_revision(void)
{
	unsigned int die_id[4] = { 0 };
	u8 package;

	omap_die_id(die_id);
	package = (die_id[2] >> 16) & 0x3;

	if (is_dra76x()) {
		switch (package) {
		case DRA762_ABZ_PACKAGE:
			*omap_si_rev = DRA762_ABZ_ES1_0;
			break;
		case DRA762_ACD_PACKAGE:
		default:
			*omap_si_rev = DRA762_ACD_ES1_0;
			break;
		}
	}
}

void omap_die_id(unsigned int *die_id)
{
	die_id[0] = readl((*ctrl)->control_std_fuse_die_id_0);
	die_id[1] = readl((*ctrl)->control_std_fuse_die_id_1);
	die_id[2] = readl((*ctrl)->control_std_fuse_die_id_2);
	die_id[3] = readl((*ctrl)->control_std_fuse_die_id_3);
}

void reset_cpu(void)
{
	u32 omap_rev = omap_revision();

	/*
	 * WARM reset is not functional in case of OMAP5430 ES1.0 soc.
	 * So use cold reset in case instead.
	 */
	if (omap_rev == OMAP5430_ES1_0)
		writel(PRM_RSTCTRL_RESET << 0x1, (*prcm)->prm_rstctrl);
	else
		writel(PRM_RSTCTRL_RESET, (*prcm)->prm_rstctrl);
}

u32 warm_reset(void)
{
	return readl((*prcm)->prm_rstst) & PRM_RSTST_WARM_RESET_MASK;
}

void setup_warmreset_time(void)
{
	u32 rst_time, rst_val;

	/*
	 * MAX value for PRM_RSTTIME[9:0]RSTTIME1 stored is 0x3ff.
	 * 0x3ff is in the no of FUNC_32K_CLK cycles. Converting cycles
	 * into microsec and passing the value.
	 */
	rst_time = usec_to_32k(CONFIG_OMAP_PLATFORM_RESET_TIME_MAX_USEC)
		<< RSTTIME1_SHIFT;

	if (rst_time > RSTTIME1_MASK)
		rst_time = RSTTIME1_MASK;

	rst_val = readl((*prcm)->prm_rsttime) & ~RSTTIME1_MASK;
	rst_val |= rst_time;
	writel(rst_val, (*prcm)->prm_rsttime);
}

void v7_arch_cp15_set_l2aux_ctrl(u32 l2auxctrl, u32 cpu_midr,
				 u32 cpu_rev_comb, u32 cpu_variant,
				 u32 cpu_rev)
{
	omap_smc1(OMAP5_SERVICE_L2ACTLR_SET, l2auxctrl);
}

void v7_arch_cp15_set_acr(u32 acr, u32 cpu_midr, u32 cpu_rev_comb,
			  u32 cpu_variant, u32 cpu_rev)
{

#ifdef CONFIG_ARM_ERRATA_801819
	/*
	 * DRA72x processors are uniprocessors and DONOT have
	 * ACP (Accelerator Coherency Port) hooked to ACE (AXI Coherency
	 * Extensions) Hence the erratum workaround is not applicable for
	 * DRA72x processors.
	 */
	if (is_dra72x())
		acr &= ~((0x3 << 23) | (0x3 << 25));
#endif
	omap_smc1(OMAP5_SERVICE_ACR_SET, acr);
}

#if defined(CONFIG_PALMAS_POWER)
__weak void board_mmc_poweron_ldo(uint voltage)
{
	palmas_mmc1_poweron_ldo(LDO1_VOLTAGE, LDO1_CTRL, voltage);
}

void vmmc_pbias_config(uint voltage)
{
	u32 value = 0;

	value = readl((*ctrl)->control_pbias);
	value &= ~SDCARD_PWRDNZ;
	writel(value, (*ctrl)->control_pbias);
	udelay(10); /* wait 10 us */
	value &= ~SDCARD_BIAS_PWRDNZ;
	writel(value, (*ctrl)->control_pbias);

	board_mmc_poweron_ldo(voltage);

	value = readl((*ctrl)->control_pbias);
	value |= SDCARD_BIAS_PWRDNZ;
	writel(value, (*ctrl)->control_pbias);
	udelay(150); /* wait 150 us */
	value |= SDCARD_PWRDNZ;
	writel(value, (*ctrl)->control_pbias);
	udelay(150); /* wait 150 us */
}
#endif
