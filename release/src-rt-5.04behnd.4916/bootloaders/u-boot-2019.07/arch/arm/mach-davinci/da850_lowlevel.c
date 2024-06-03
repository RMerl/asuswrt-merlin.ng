// SPDX-License-Identifier: GPL-2.0+
/*
 * SoC-specific lowlevel code for DA850
 *
 * Copyright (C) 2011
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 */
#include <common.h>
#include <nand.h>
#include <ns16550.h>
#include <post.h>
#include <asm/arch/da850_lowlevel.h>
#include <asm/arch/hardware.h>
#include <asm/arch/davinci_misc.h>
#include <asm/arch/ddr2_defs.h>
#include <asm/ti-common/davinci_nand.h>
#include <asm/arch/pll_defs.h>

void davinci_enable_uart0(void)
{
	lpsc_on(DAVINCI_LPSC_UART0);

	/* Bringup UART0 out of reset */
	REG(UART0_PWREMU_MGMT) = 0x00006001;
}

#if defined(CONFIG_SYS_DA850_PLL_INIT)
static void da850_waitloop(unsigned long loopcnt)
{
	unsigned long	i;

	for (i = 0; i < loopcnt; i++)
		asm("   NOP");
}

static int da850_pll_init(struct davinci_pllc_regs *reg, unsigned long pllmult)
{
	if (reg == davinci_pllc0_regs)
		/* Unlock PLL registers. */
		clrbits_le32(&davinci_syscfg_regs->cfgchip0, PLL_MASTER_LOCK);

	/*
	 * Set PLLENSRC '0',bit 5, PLL Enable(PLLEN) selection is controlled
	 * through MMR
	 */
	clrbits_le32(&reg->pllctl, PLLCTL_PLLENSRC);
	/* PLLCTL.EXTCLKSRC bit 9 should be left at 0 for Freon */
	clrbits_le32(&reg->pllctl, PLLCTL_EXTCLKSRC);

	/* Set PLLEN=0 => PLL BYPASS MODE */
	clrbits_le32(&reg->pllctl, PLLCTL_PLLEN);

	da850_waitloop(150);

	if (reg == davinci_pllc0_regs) {
		/*
		 * Select the Clock Mode bit 8 as External Clock or On Chip
		 * Oscilator
		 */
		dv_maskbits(&reg->pllctl, ~PLLCTL_RES_9);
		setbits_le32(&reg->pllctl,
			(CONFIG_SYS_DV_CLKMODE << PLLCTL_CLOCK_MODE_SHIFT));
	}

	/* Clear PLLRST bit to reset the PLL */
	clrbits_le32(&reg->pllctl, PLLCTL_PLLRST);

	/* Disable the PLL output */
	setbits_le32(&reg->pllctl, PLLCTL_PLLDIS);

	/* PLL initialization sequence */
	/*
	 * Power up the PLL- PWRDN bit set to 0 to bring the PLL out of
	 * power down bit
	 */
	clrbits_le32(&reg->pllctl, PLLCTL_PLLPWRDN);

	/* Enable the PLL from Disable Mode PLLDIS bit to 0 */
	clrbits_le32(&reg->pllctl, PLLCTL_PLLDIS);

#if defined(CONFIG_SYS_DA850_PLL0_PREDIV)
	/* program the prediv */
	if (reg == davinci_pllc0_regs && CONFIG_SYS_DA850_PLL0_PREDIV)
		writel((PLL_DIVEN | CONFIG_SYS_DA850_PLL0_PREDIV),
			&reg->prediv);
#endif

	/* Program the required multiplier value in PLLM */
	writel(pllmult, &reg->pllm);

	/* program the postdiv */
	if (reg == davinci_pllc0_regs)
		writel((PLL_POSTDEN | CONFIG_SYS_DA850_PLL0_POSTDIV),
			&reg->postdiv);
	else
		writel((PLL_POSTDEN | CONFIG_SYS_DA850_PLL1_POSTDIV),
			&reg->postdiv);

	/*
	 * Check for the GOSTAT bit in PLLSTAT to clear to 0 to indicate that
	 * no GO operation is currently in progress
	 */
	while ((readl(&reg->pllstat) & PLLCMD_GOSTAT) == PLLCMD_GOSTAT)
		;

	if (reg == davinci_pllc0_regs) {
		writel(CONFIG_SYS_DA850_PLL0_PLLDIV1, &reg->plldiv1);
		writel(CONFIG_SYS_DA850_PLL0_PLLDIV2, &reg->plldiv2);
		writel(CONFIG_SYS_DA850_PLL0_PLLDIV3, &reg->plldiv3);
		writel(CONFIG_SYS_DA850_PLL0_PLLDIV4, &reg->plldiv4);
		writel(CONFIG_SYS_DA850_PLL0_PLLDIV5, &reg->plldiv5);
		writel(CONFIG_SYS_DA850_PLL0_PLLDIV6, &reg->plldiv6);
		writel(CONFIG_SYS_DA850_PLL0_PLLDIV7, &reg->plldiv7);
	} else {
		writel(CONFIG_SYS_DA850_PLL1_PLLDIV1, &reg->plldiv1);
		writel(CONFIG_SYS_DA850_PLL1_PLLDIV2, &reg->plldiv2);
		writel(CONFIG_SYS_DA850_PLL1_PLLDIV3, &reg->plldiv3);
	}

	/*
	 * Set the GOSET bit in PLLCMD to 1 to initiate a new divider
	 * transition.
	 */
	setbits_le32(&reg->pllcmd, PLLCMD_GOSTAT);

	/*
	 * Wait for the GOSTAT bit in PLLSTAT to clear to 0
	 * (completion of phase alignment).
	 */
	while ((readl(&reg->pllstat) & PLLCMD_GOSTAT) == PLLCMD_GOSTAT)
		;

	/* Wait for PLL to reset properly. See PLL spec for PLL reset time */
	da850_waitloop(200);

	/* Set the PLLRST bit in PLLCTL to 1 to bring the PLL out of reset */
	setbits_le32(&reg->pllctl, PLLCTL_PLLRST);

	/* Wait for PLL to lock. See PLL spec for PLL lock time */
	da850_waitloop(2400);

	/*
	 * Set the PLLEN bit in PLLCTL to 1 to remove the PLL from bypass
	 * mode
	 */
	setbits_le32(&reg->pllctl, PLLCTL_PLLEN);


	/*
	 * clear EMIFA and EMIFB clock source settings, let them
	 * run off SYSCLK
	 */
	if (reg == davinci_pllc0_regs)
		dv_maskbits(&davinci_syscfg_regs->cfgchip3,
			~(PLL_SCSCFG3_DIV45PENA | PLL_SCSCFG3_EMA_CLKSRC));

	return 0;
}
#endif /* CONFIG_SYS_DA850_PLL_INIT */

#if defined(CONFIG_SYS_DA850_DDR_INIT)
static int da850_ddr_setup(void)
{
	unsigned long	tmp;

	/* Enable the Clock to DDR2/mDDR */
	lpsc_on(DAVINCI_LPSC_DDR_EMIF);

	tmp = readl(&davinci_syscfg1_regs->vtpio_ctl);
	if ((tmp & VTP_POWERDWN) == VTP_POWERDWN) {
		/* Begin VTP Calibration */
		clrbits_le32(&davinci_syscfg1_regs->vtpio_ctl, VTP_POWERDWN);
		clrbits_le32(&davinci_syscfg1_regs->vtpio_ctl, VTP_LOCK);
		setbits_le32(&davinci_syscfg1_regs->vtpio_ctl, VTP_CLKRZ);
		clrbits_le32(&davinci_syscfg1_regs->vtpio_ctl, VTP_CLKRZ);
		setbits_le32(&davinci_syscfg1_regs->vtpio_ctl, VTP_CLKRZ);

		/* Polling READY bit to see when VTP calibration is done */
		tmp = readl(&davinci_syscfg1_regs->vtpio_ctl);
		while ((tmp & VTP_READY) != VTP_READY)
			tmp = readl(&davinci_syscfg1_regs->vtpio_ctl);

		setbits_le32(&davinci_syscfg1_regs->vtpio_ctl, VTP_LOCK);
		setbits_le32(&davinci_syscfg1_regs->vtpio_ctl, VTP_POWERDWN);
	}
	setbits_le32(&davinci_syscfg1_regs->vtpio_ctl, VTP_IOPWRDWN);
	writel(CONFIG_SYS_DA850_DDR2_DDRPHYCR, &dv_ddr2_regs_ctrl->ddrphycr);

	if (CONFIG_SYS_DA850_DDR2_SDBCR & (1 << DV_DDR_SDCR_DDR2EN_SHIFT)) {
		/* DDR2 */
		clrbits_le32(&davinci_syscfg1_regs->ddr_slew,
			(1 << DDR_SLEW_DDR_PDENA_BIT) |
			(1 << DDR_SLEW_CMOSEN_BIT));
	} else {
		/* MOBILE DDR */
		setbits_le32(&davinci_syscfg1_regs->ddr_slew,
			(1 << DDR_SLEW_DDR_PDENA_BIT) |
			(1 << DDR_SLEW_CMOSEN_BIT));
	}

	/*
	 * SDRAM Configuration Register (SDCR):
	 * First set the BOOTUNLOCK bit to make configuration bits
	 * writeable.
	 */
	setbits_le32(&dv_ddr2_regs_ctrl->sdbcr, DV_DDR_BOOTUNLOCK);

	/*
	 * Write the new value of these bits and clear BOOTUNLOCK.
	 * At the same time, set the TIMUNLOCK bit to allow changing
	 * the timing registers
	 */
	tmp = CONFIG_SYS_DA850_DDR2_SDBCR;
	tmp &= ~DV_DDR_BOOTUNLOCK;
	tmp |= DV_DDR_TIMUNLOCK;
	writel(tmp, &dv_ddr2_regs_ctrl->sdbcr);

	/* write memory configuration and timing */
	if (!(CONFIG_SYS_DA850_DDR2_SDBCR & (1 << DV_DDR_SDCR_DDR2EN_SHIFT))) {
		/* MOBILE DDR only*/
		writel(CONFIG_SYS_DA850_DDR2_SDBCR2,
			&dv_ddr2_regs_ctrl->sdbcr2);
	}
	writel(CONFIG_SYS_DA850_DDR2_SDTIMR, &dv_ddr2_regs_ctrl->sdtimr);
	writel(CONFIG_SYS_DA850_DDR2_SDTIMR2, &dv_ddr2_regs_ctrl->sdtimr2);

	/* clear the TIMUNLOCK bit and write the value of the CL field */
	tmp &= ~DV_DDR_TIMUNLOCK;
	writel(tmp, &dv_ddr2_regs_ctrl->sdbcr);

	/*
	 * LPMODEN and MCLKSTOPEN must be set!
	 * Without this bits set, PSC don;t switch states !!
	 */
	writel(CONFIG_SYS_DA850_DDR2_SDRCR |
		(1 << DV_DDR_SRCR_LPMODEN_SHIFT) |
		(1 << DV_DDR_SRCR_MCLKSTOPEN_SHIFT),
		&dv_ddr2_regs_ctrl->sdrcr);

	/* SyncReset the Clock to EMIF3A SDRAM */
	lpsc_syncreset(DAVINCI_LPSC_DDR_EMIF);
	/* Enable the Clock to EMIF3A SDRAM */
	lpsc_on(DAVINCI_LPSC_DDR_EMIF);

	/* disable self refresh */
	clrbits_le32(&dv_ddr2_regs_ctrl->sdrcr,
		DV_DDR_SDRCR_LPMODEN | DV_DDR_SDRCR_MCLKSTOPEN);
	writel(CONFIG_SYS_DA850_DDR2_PBBPR, &dv_ddr2_regs_ctrl->pbbpr);

	return 0;
}
#endif /* CONFIG_SYS_DA850_DDR_INIT */

__attribute__((weak))
void board_gpio_init(void)
{
	return;
}

int arch_cpu_init(void)
{
	/* Unlock kick registers */
	writel(DV_SYSCFG_KICK0_UNLOCK, &davinci_syscfg_regs->kick0);
	writel(DV_SYSCFG_KICK1_UNLOCK, &davinci_syscfg_regs->kick1);

	dv_maskbits(&davinci_syscfg_regs->suspsrc,
		CONFIG_SYS_DA850_SYSCFG_SUSPSRC);

	/* configure pinmux settings */
	if (davinci_configure_pin_mux_items(pinmuxes, pinmuxes_size))
		return 1;

#if defined(CONFIG_SYS_DA850_PLL_INIT)
	/* PLL setup */
	da850_pll_init(davinci_pllc0_regs, CONFIG_SYS_DA850_PLL0_PLLM);
	da850_pll_init(davinci_pllc1_regs, CONFIG_SYS_DA850_PLL1_PLLM);
#endif
	/* setup CSn config */
#if defined(CONFIG_SYS_DA850_CS2CFG)
	writel(CONFIG_SYS_DA850_CS2CFG, &davinci_emif_regs->ab1cr);
#endif
#if defined(CONFIG_SYS_DA850_CS3CFG)
	writel(CONFIG_SYS_DA850_CS3CFG, &davinci_emif_regs->ab2cr);
#endif

	da8xx_configure_lpsc_items(lpsc, lpsc_size);

	/* GPIO setup */
	board_gpio_init();

#if !CONFIG_IS_ENABLED(DM_SERIAL)
	NS16550_init((NS16550_t)(CONFIG_SYS_NS16550_COM1),
			CONFIG_SYS_NS16550_CLK / 16 / CONFIG_BAUDRATE);
#endif
	/*
	 * Fix Power and Emulation Management Register
	 * see sprufw3a.pdf page 37 Table 24
	 */
	writel((DAVINCI_UART_PWREMU_MGMT_FREE | DAVINCI_UART_PWREMU_MGMT_URRST |
		DAVINCI_UART_PWREMU_MGMT_UTRST),
#if (CONFIG_SYS_NS16550_COM1 == DAVINCI_UART0_BASE)
	       &davinci_uart0_ctrl_regs->pwremu_mgmt);
#else
	       &davinci_uart2_ctrl_regs->pwremu_mgmt);
#endif

#if defined(CONFIG_SYS_DA850_DDR_INIT)
	da850_ddr_setup();
#endif

	return 0;
}
