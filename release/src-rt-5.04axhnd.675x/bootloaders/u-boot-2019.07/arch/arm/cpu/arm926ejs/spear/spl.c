// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2011
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 *
 * Copyright (C) 2012 Stefan Roese <sr@denx.de>
 */

#include <common.h>
#include <spl.h>
#include <version.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/arch/spr_defs.h>
#include <asm/arch/spr_misc.h>
#include <asm/arch/spr_syscntl.h>
#include <linux/mtd/st_smi.h>

static void ddr_clock_init(void)
{
	struct misc_regs *misc_p = (struct misc_regs *)CONFIG_SPEAR_MISCBASE;
	u32 clkenb, ddrpll;

	clkenb = readl(&misc_p->periph1_clken);
	clkenb &= ~PERIPH_MPMCMSK;
	clkenb |= PERIPH_MPMC_WE;

	/* Intentionally done twice */
	writel(clkenb, &misc_p->periph1_clken);
	writel(clkenb, &misc_p->periph1_clken);

	ddrpll = readl(&misc_p->pll_ctr_reg);
	ddrpll &= ~MEM_CLK_SEL_MSK;
#if (CONFIG_DDR_HCLK)
	ddrpll |= MEM_CLK_HCLK;
#elif (CONFIG_DDR_2HCLK)
	ddrpll |= MEM_CLK_2HCLK;
#elif (CONFIG_DDR_PLL2)
	ddrpll |= MEM_CLK_PLL2;
#else
#error "please define one of CONFIG_DDR_(HCLK|2HCLK|PLL2)"
#endif
	writel(ddrpll, &misc_p->pll_ctr_reg);

	writel(readl(&misc_p->periph1_clken) | PERIPH_MPMC_EN,
			&misc_p->periph1_clken);
}

static void mpmc_init_values(void)
{
	u32 i;
	u32 *mpmc_reg_p = (u32 *)CONFIG_SPEAR_MPMCBASE;
	u32 *mpmc_val_p = &mpmc_conf_vals[0];

	for (i = 0; i < CONFIG_SPEAR_MPMCREGS; i++, mpmc_reg_p++, mpmc_val_p++)
		writel(*mpmc_val_p, mpmc_reg_p);

	mpmc_reg_p = (u32 *)CONFIG_SPEAR_MPMCBASE;

	/*
	 * MPMC controller start
	 * MPMC waiting for DLLLOCKREG high
	 */
	writel(0x01000100, &mpmc_reg_p[7]);

	while (!(readl(&mpmc_reg_p[3]) & 0x10000))
		;
}

static void mpmc_init(void)
{
	/* Clock related settings for DDR */
	ddr_clock_init();

	/*
	 * DDR pad register bits are different for different SoCs
	 * Compensation values are also handled separately
	 */
	plat_ddr_init();

	/* Initialize mpmc register values */
	mpmc_init_values();
}

static void pll_init(void)
{
	struct misc_regs *misc_p = (struct misc_regs *)CONFIG_SPEAR_MISCBASE;

	/* Initialize PLLs */
	writel(FREQ_332, &misc_p->pll1_frq);
	writel(0x1C0A, &misc_p->pll1_cntl);
	writel(0x1C0E, &misc_p->pll1_cntl);
	writel(0x1C06, &misc_p->pll1_cntl);
	writel(0x1C0E, &misc_p->pll1_cntl);

	writel(FREQ_332, &misc_p->pll2_frq);
	writel(0x1C0A, &misc_p->pll2_cntl);
	writel(0x1C0E, &misc_p->pll2_cntl);
	writel(0x1C06, &misc_p->pll2_cntl);
	writel(0x1C0E, &misc_p->pll2_cntl);

	/* wait for pll locks */
	while (!(readl(&misc_p->pll1_cntl) & 0x1))
		;
	while (!(readl(&misc_p->pll2_cntl) & 0x1))
		;
}

static void mac_init(void)
{
	struct misc_regs *misc_p = (struct misc_regs *)CONFIG_SPEAR_MISCBASE;

	writel(readl(&misc_p->periph1_clken) & (~PERIPH_GMAC),
			&misc_p->periph1_clken);

	writel(SYNTH23, &misc_p->gmac_synth_clk);

	switch (get_socrev()) {
	case SOC_SPEAR600_AA:
	case SOC_SPEAR600_AB:
	case SOC_SPEAR600_BA:
	case SOC_SPEAR600_BB:
	case SOC_SPEAR600_BC:
	case SOC_SPEAR600_BD:
		writel(0x0, &misc_p->gmac_ctr_reg);
		break;

	case SOC_SPEAR300:
	case SOC_SPEAR310:
	case SOC_SPEAR320:
		writel(0x4, &misc_p->gmac_ctr_reg);
		break;
	}

	writel(readl(&misc_p->periph1_clken) | PERIPH_GMAC,
			&misc_p->periph1_clken);

	writel(readl(&misc_p->periph1_rst) | PERIPH_GMAC,
			&misc_p->periph1_rst);
	writel(readl(&misc_p->periph1_rst) & (~PERIPH_GMAC),
			&misc_p->periph1_rst);
}

static void sys_init(void)
{
	struct misc_regs *misc_p = (struct misc_regs *)CONFIG_SPEAR_MISCBASE;
	struct syscntl_regs *syscntl_p =
		(struct syscntl_regs *)CONFIG_SPEAR_SYSCNTLBASE;

	/* Set system state to SLOW */
	writel(SLOW, &syscntl_p->scctrl);
	writel(PLL_TIM << 3, &syscntl_p->scpllctrl);

	/* Initialize PLLs */
	pll_init();

	/*
	 * Ethernet configuration
	 * To be done only if the tftp boot is not selected already
	 * Boot code ensures the correct configuration in tftp booting
	 */
	if (!tftp_boot_selected())
		mac_init();

	writel(RTC_DISABLE | PLLTIMEEN, &misc_p->periph_clk_cfg);
	writel(0x555, &misc_p->amba_clk_cfg);

	writel(NORMAL, &syscntl_p->scctrl);

	/* Wait for system to switch to normal mode */
	while (((readl(&syscntl_p->scctrl) >> MODE_SHIFT) & MODE_MASK)
		!= NORMAL)
		;
}

/*
 * get_socrev
 *
 * Get SoC Revision.
 * @return SOC_SPEARXXX
 */
int get_socrev(void)
{
#if defined(CONFIG_SPEAR600)
	struct misc_regs *misc_p = (struct misc_regs *)CONFIG_SPEAR_MISCBASE;
	u32 soc_id = readl(&misc_p->soc_core_id);
	u32 pri_socid = (soc_id >> SOC_PRI_SHFT) & 0xFF;
	u32 sec_socid = (soc_id >> SOC_SEC_SHFT) & 0xFF;

	if ((pri_socid == 'B') && (sec_socid == 'B'))
		return SOC_SPEAR600_BB;
	else if ((pri_socid == 'B') && (sec_socid == 'C'))
		return SOC_SPEAR600_BC;
	else if ((pri_socid == 'B') && (sec_socid == 'D'))
		return SOC_SPEAR600_BD;
	else if (soc_id == 0)
		return SOC_SPEAR600_BA;
	else
		return SOC_SPEAR_NA;
#elif defined(CONFIG_SPEAR300)
	return SOC_SPEAR300;
#elif defined(CONFIG_SPEAR310)
	return SOC_SPEAR310;
#elif defined(CONFIG_SPEAR320)
	return SOC_SPEAR320;
#endif
}

/*
 * SNOR (Serial NOR flash) related functions
 */
static void snor_init(void)
{
	struct smi_regs *const smicntl =
		(struct smi_regs * const)CONFIG_SYS_SMI_BASE;

	/* Setting the fast mode values. SMI working at 166/4 = 41.5 MHz */
	writel(HOLD1 | FAST_MODE | BANK_EN | DSEL_TIME | PRESCAL4,
	       &smicntl->smi_cr1);
}

u32 spl_boot_device(void)
{
	u32 mode = 0;

	/* Currently only SNOR is supported as the only */
	if (snor_boot_selected()) {
		/* SNOR-SMI initialization */
		snor_init();

		mode = BOOT_DEVICE_NOR;
	}

	return mode;
}

void board_init_f(ulong dummy)
{
	struct misc_regs *misc_p = (struct misc_regs *)CONFIG_SPEAR_MISCBASE;

	/* Initialize PLLs */
	sys_init();

	preloader_console_init();
	arch_cpu_init();

	/* Enable IPs (release reset) */
	writel(PERIPH_RST_ALL, &misc_p->periph1_rst);

	/* Initialize MPMC */
	puts("Configure DDR\n");
	mpmc_init();
	spear_late_init();

	board_init_r(NULL, 0);
}
