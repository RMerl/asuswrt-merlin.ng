// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2018 Microsemi Corporation
 */

#include <common.h>

#include <asm/sections.h>
#include <asm/io.h>

#include <asm/reboot.h>

void _machine_restart(void)
{
#if defined(CONFIG_SOC_JR2) || defined(CONFIG_SOC_SERVALT)
	register u32 reg = readl(BASE_CFG + ICPU_GENERAL_CTRL);
	/* Set owner */
	reg &= ~ICPU_GENERAL_CTRL_IF_SI_OWNER_M;
	reg |= ICPU_GENERAL_CTRL_IF_SI_OWNER(1);
	/* Set boot mode */
	reg |= ICPU_GENERAL_CTRL_BOOT_MODE_ENA;
	writel(reg, BASE_CFG + ICPU_GENERAL_CTRL);
	/* Read back in order to make BOOT mode setting active */
	reg = readl(BASE_CFG + ICPU_GENERAL_CTRL);
	/* Reset CPU only - still executing _here_. but from cache */
	writel(readl(BASE_CFG + ICPU_RESET) |
	       ICPU_RESET_CORE_RST_CPU_ONLY |
	       ICPU_RESET_CORE_RST_FORCE,
	       BASE_CFG + ICPU_RESET);
#elif defined(CONFIG_SOC_SERVAL)
	register unsigned long i;

	/* Prevent VCore-III from being reset with a global reset */
	writel(ICPU_RESET_CORE_RST_PROTECT, BASE_CFG + ICPU_RESET);

	/* Do global reset */
	writel(PERF_SOFT_RST_SOFT_CHIP_RST, BASE_DEVCPU_GCB + PERF_SOFT_RST);

	for (i = 0; i < 2000; i++)
		;

	/* Power down DDR for clean DDR re-training */
	writel(readl(BASE_CFG + ICPU_MEMCTRL_CTRL) |
	       ICPU_MEMCTRL_CTRL_PWR_DOWN,
	       BASE_CFG + ICPU_MEMCTRL_CTRL);

	while (!(readl(BASE_CFG + ICPU_MEMCTRL_STAT) &
		 ICPU_MEMCTRL_STAT_PWR_DOWN_ACK))
		;

	/* Reset VCore-III, only. */
	writel(ICPU_RESET_CORE_RST_FORCE, BASE_CFG + ICPU_RESET);
#else		/* Luton || Ocelot */
	register u32 resetbits = PERF_SOFT_RST_SOFT_CHIP_RST;
	(void)readl(BASE_DEVCPU_GCB + PERF_SOFT_RST);

	/* Make sure VCore is NOT protected from reset */
	clrbits_le32(BASE_CFG + ICPU_RESET, ICPU_RESET_CORE_RST_PROTECT);

	/* Change to SPI bitbang for SPI reset workaround... */
	writel(ICPU_SW_MODE_SW_SPI_CS_OE(1) | ICPU_SW_MODE_SW_SPI_CS(1) |
	       ICPU_SW_MODE_SW_PIN_CTRL_MODE, BASE_CFG + ICPU_SW_MODE);

	/* Do the global reset */
	writel(resetbits, BASE_DEVCPU_GCB + PERF_SOFT_RST);
#endif

	while (1)
		; /* NOP */
}
