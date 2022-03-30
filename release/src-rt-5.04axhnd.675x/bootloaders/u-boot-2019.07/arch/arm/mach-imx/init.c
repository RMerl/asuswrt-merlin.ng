// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2015 Freescale Semiconductor, Inc.
 */

#include <asm/io.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/arch/crm_regs.h>

void init_aips(void)
{
	struct aipstz_regs *aips1, *aips2, *aips3;

	aips1 = (struct aipstz_regs *)AIPS1_BASE_ADDR;
	aips2 = (struct aipstz_regs *)AIPS2_BASE_ADDR;
	aips3 = (struct aipstz_regs *)AIPS3_BASE_ADDR;

	/*
	 * Set all MPROTx to be non-bufferable, trusted for R/W,
	 * not forced to user-mode.
	 */
	writel(0x77777777, &aips1->mprot0);
	writel(0x77777777, &aips1->mprot1);
	writel(0x77777777, &aips2->mprot0);
	writel(0x77777777, &aips2->mprot1);

	/*
	 * Set all OPACRx to be non-bufferable, not require
	 * supervisor privilege level for access,allow for
	 * write access and untrusted master access.
	 */
	writel(0x00000000, &aips1->opacr0);
	writel(0x00000000, &aips1->opacr1);
	writel(0x00000000, &aips1->opacr2);
	writel(0x00000000, &aips1->opacr3);
	writel(0x00000000, &aips1->opacr4);
	writel(0x00000000, &aips2->opacr0);
	writel(0x00000000, &aips2->opacr1);
	writel(0x00000000, &aips2->opacr2);
	writel(0x00000000, &aips2->opacr3);
	writel(0x00000000, &aips2->opacr4);

	if (is_mx6ull() || is_mx6sx() || is_mx7()) {
		/*
		 * Set all MPROTx to be non-bufferable, trusted for R/W,
		 * not forced to user-mode.
		 */
		writel(0x77777777, &aips3->mprot0);
		writel(0x77777777, &aips3->mprot1);

		/*
		 * Set all OPACRx to be non-bufferable, not require
		 * supervisor privilege level for access,allow for
		 * write access and untrusted master access.
		 */
		writel(0x00000000, &aips3->opacr0);
		writel(0x00000000, &aips3->opacr1);
		writel(0x00000000, &aips3->opacr2);
		writel(0x00000000, &aips3->opacr3);
		writel(0x00000000, &aips3->opacr4);
	}
}

void imx_wdog_disable_powerdown(void)
{
	struct wdog_regs *wdog1 = (struct wdog_regs *)WDOG1_BASE_ADDR;
	struct wdog_regs *wdog2 = (struct wdog_regs *)WDOG2_BASE_ADDR;
	struct wdog_regs *wdog3 = (struct wdog_regs *)WDOG3_BASE_ADDR;
#ifdef CONFIG_MX7D
	struct wdog_regs *wdog4 = (struct wdog_regs *)WDOG4_BASE_ADDR;
#endif

	/* Write to the PDE (Power Down Enable) bit */
	writew(0, &wdog1->wmcr);
	writew(0, &wdog2->wmcr);

	if (is_mx6sx() || is_mx6ul() || is_mx6ull() || is_mx7())
		writew(0, &wdog3->wmcr);
#ifdef CONFIG_MX7D
	writew(0, &wdog4->wmcr);
#endif
}

#define SRC_SCR_WARM_RESET_ENABLE	0

void init_src(void)
{
	struct src *src_regs = (struct src *)SRC_BASE_ADDR;
	u32 val;

	/*
	 * force warm reset sources to generate cold reset
	 * for a more reliable restart
	 */
	val = readl(&src_regs->scr);
	val &= ~(1 << SRC_SCR_WARM_RESET_ENABLE);
	writel(val, &src_regs->scr);
}

#ifdef CONFIG_CMD_BMODE
void boot_mode_apply(unsigned cfg_val)
{
	unsigned reg;
	struct src *psrc = (struct src *)SRC_BASE_ADDR;
	writel(cfg_val, &psrc->gpr9);
	reg = readl(&psrc->gpr10);
	if (cfg_val)
		reg |= 1 << 28;
	else
		reg &= ~(1 << 28);
	writel(reg, &psrc->gpr10);
}
#endif

#if defined(CONFIG_MX6)
u32 imx6_src_get_boot_mode(void)
{
	if (imx6_is_bmode_from_gpr9())
		return readl(&src_base->gpr9);
	else
		return readl(&src_base->sbmr1);
}
#endif
