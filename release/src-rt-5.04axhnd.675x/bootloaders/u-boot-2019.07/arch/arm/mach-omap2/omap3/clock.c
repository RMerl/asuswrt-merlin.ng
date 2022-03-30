// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008
 * Texas Instruments, <www.ti.com>
 *
 * Author :
 *      Manikandan Pillai <mani.pillai@ti.com>
 *
 * Derived from Beagle Board and OMAP3 SDP code by
 *      Richard Woodruff <r-woodruff2@ti.com>
 *      Syed Mohammed Khasim <khasim@ti.com>
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/clocks_omap3.h>
#include <asm/arch/mem.h>
#include <asm/arch/sys_proto.h>
#include <environment.h>
#include <command.h>

/******************************************************************************
 * get_sys_clk_speed() - determine reference oscillator speed
 *                       based on known 32kHz clock and gptimer.
 *****************************************************************************/
u32 get_osc_clk_speed(void)
{
	u32 start, cstart, cend, cdiff, cdiv, val;
	struct prcm *prcm_base = (struct prcm *)PRCM_BASE;
	struct prm *prm_base = (struct prm *)PRM_BASE;
	struct gptimer *gpt1_base = (struct gptimer *)OMAP34XX_GPT1;
	struct s32ktimer *s32k_base = (struct s32ktimer *)SYNC_32KTIMER_BASE;

	val = readl(&prm_base->clksrc_ctrl);

	if (val & SYSCLKDIV_2)
		cdiv = 2;
	else
		cdiv = 1;

	/* enable timer2 */
	val = readl(&prcm_base->clksel_wkup) | CLKSEL_GPT1;

	/* select sys_clk for GPT1 */
	writel(val, &prcm_base->clksel_wkup);

	/* Enable I and F Clocks for GPT1 */
	val = readl(&prcm_base->iclken_wkup) | EN_GPT1 | EN_32KSYNC;
	writel(val, &prcm_base->iclken_wkup);

	val = readl(&prcm_base->fclken_wkup) | EN_GPT1;
	writel(val, &prcm_base->fclken_wkup);

	writel(0, &gpt1_base->tldr);		/* start counting at 0 */
	writel(GPT_EN, &gpt1_base->tclr);	/* enable clock */

	/* enable 32kHz source, determine sys_clk via gauging */

	/* start time in 20 cycles */
	start = 20 + readl(&s32k_base->s32k_cr);

	/* dead loop till start time */
	while (readl(&s32k_base->s32k_cr) < start);

	/* get start sys_clk count */
	cstart = readl(&gpt1_base->tcrr);

	/* wait for 40 cycles */
	while (readl(&s32k_base->s32k_cr) < (start + 20)) ;
	cend = readl(&gpt1_base->tcrr);		/* get end sys_clk count */
	cdiff = cend - cstart;			/* get elapsed ticks */
	cdiff *= cdiv;

	/* based on number of ticks assign speed */
	if (cdiff > 19000)
		return S38_4M;
	else if (cdiff > 15200)
		return S26M;
	else if (cdiff > 13000)
		return S24M;
	else if (cdiff > 9000)
		return S19_2M;
	else if (cdiff > 7600)
		return S13M;
	else
		return S12M;
}

/******************************************************************************
 * get_sys_clkin_sel() - returns the sys_clkin_sel field value based on
 *                       input oscillator clock frequency.
 *****************************************************************************/
void get_sys_clkin_sel(u32 osc_clk, u32 *sys_clkin_sel)
{
	switch(osc_clk) {
	case S38_4M:
		*sys_clkin_sel = 4;
		break;
	case S26M:
		*sys_clkin_sel = 3;
		break;
	case S19_2M:
		*sys_clkin_sel = 2;
		break;
	case S13M:
		*sys_clkin_sel = 1;
		break;
	case S12M:
	default:
		*sys_clkin_sel = 0;
	}
}

/*
 * OMAP34XX/35XX specific functions
 */

static void dpll3_init_34xx(u32 sil_index, u32 clk_index)
{
	struct prcm *prcm_base = (struct prcm *)PRCM_BASE;
	dpll_param *ptr = (dpll_param *) get_core_dpll_param();
	void (*f_lock_pll) (u32, u32, u32, u32);
	int xip_safe, p0, p1, p2, p3;

	xip_safe = is_running_in_sram();

	/* Moving to the right sysclk and ES rev base */
	ptr = ptr + (3 * clk_index) + sil_index;

	if (xip_safe) {
		/*
		 * CORE DPLL
		 */
		clrsetbits_le32(&prcm_base->clken_pll,
				0x00000007, PLL_FAST_RELOCK_BYPASS);
		wait_on_value(ST_CORE_CLK, 0, &prcm_base->idlest_ckgen,
				LDELAY);

		/*
		 * For OMAP3 ES1.0 Errata 1.50, default value directly doesn't
		 * work. write another value and then default value.
		 */

		/* CM_CLKSEL1_EMU[DIV_DPLL3] */
		clrsetbits_le32(&prcm_base->clksel1_emu,
				0x001F0000, (CORE_M3X2 + 1) << 16) ;
		clrsetbits_le32(&prcm_base->clksel1_emu,
				0x001F0000, CORE_M3X2 << 16);

		/* M2 (CORE_DPLL_CLKOUT_DIV): CM_CLKSEL1_PLL[27:31] */
		clrsetbits_le32(&prcm_base->clksel1_pll,
				0xF8000000, ptr->m2 << 27);

		/* M (CORE_DPLL_MULT): CM_CLKSEL1_PLL[16:26] */
		clrsetbits_le32(&prcm_base->clksel1_pll,
				0x07FF0000, ptr->m << 16);

		/* N (CORE_DPLL_DIV): CM_CLKSEL1_PLL[8:14] */
		clrsetbits_le32(&prcm_base->clksel1_pll,
				0x00007F00, ptr->n << 8);

		/* Source is the CM_96M_FCLK: CM_CLKSEL1_PLL[6] */
		clrbits_le32(&prcm_base->clksel1_pll, 0x00000040);

		/* SSI */
		clrsetbits_le32(&prcm_base->clksel_core,
				0x00000F00, CORE_SSI_DIV << 8);
		/* FSUSB */
		clrsetbits_le32(&prcm_base->clksel_core,
				0x00000030, CORE_FUSB_DIV << 4);
		/* L4 */
		clrsetbits_le32(&prcm_base->clksel_core,
				0x0000000C, CORE_L4_DIV << 2);
		/* L3 */
		clrsetbits_le32(&prcm_base->clksel_core,
				0x00000003, CORE_L3_DIV);
		/* GFX */
		clrsetbits_le32(&prcm_base->clksel_gfx,
				0x00000007, GFX_DIV);
		/* RESET MGR */
		clrsetbits_le32(&prcm_base->clksel_wkup,
				0x00000006, WKUP_RSM << 1);
		/* FREQSEL (CORE_DPLL_FREQSEL): CM_CLKEN_PLL[4:7] */
		clrsetbits_le32(&prcm_base->clken_pll,
				0x000000F0, ptr->fsel << 4);
		/* LOCK MODE */
		clrsetbits_le32(&prcm_base->clken_pll,
				0x00000007, PLL_LOCK);

		wait_on_value(ST_CORE_CLK, 1, &prcm_base->idlest_ckgen,
				LDELAY);
	} else if (is_running_in_flash()) {
		/*
		 * if running from flash, jump to small relocated code
		 * area in SRAM.
		 */
		f_lock_pll = (void *) (SRAM_CLK_CODE);

		p0 = readl(&prcm_base->clken_pll);
		clrsetbits_le32(&p0, 0x00000007, PLL_FAST_RELOCK_BYPASS);
		/* FREQSEL (CORE_DPLL_FREQSEL): CM_CLKEN_PLL[4:7] */
		clrsetbits_le32(&p0, 0x000000F0, ptr->fsel << 4);

		p1 = readl(&prcm_base->clksel1_pll);
		/* M2 (CORE_DPLL_CLKOUT_DIV): CM_CLKSEL1_PLL[27:31] */
		clrsetbits_le32(&p1, 0xF8000000, ptr->m2 << 27);
		/* M (CORE_DPLL_MULT): CM_CLKSEL1_PLL[16:26] */
		clrsetbits_le32(&p1, 0x07FF0000, ptr->m << 16);
		/* N (CORE_DPLL_DIV): CM_CLKSEL1_PLL[8:14] */
		clrsetbits_le32(&p1, 0x00007F00, ptr->n << 8);
		/* Source is the CM_96M_FCLK: CM_CLKSEL1_PLL[6] */
		clrbits_le32(&p1, 0x00000040);

		p2 = readl(&prcm_base->clksel_core);
		/* SSI */
		clrsetbits_le32(&p2, 0x00000F00, CORE_SSI_DIV << 8);
		/* FSUSB */
		clrsetbits_le32(&p2, 0x00000030, CORE_FUSB_DIV << 4);
		/* L4 */
		clrsetbits_le32(&p2, 0x0000000C, CORE_L4_DIV << 2);
		/* L3 */
		clrsetbits_le32(&p2, 0x00000003, CORE_L3_DIV);

		p3 = (u32)&prcm_base->idlest_ckgen;

		(*f_lock_pll) (p0, p1, p2, p3);
	}
}

static void dpll4_init_34xx(u32 sil_index, u32 clk_index)
{
	struct prcm *prcm_base = (struct prcm *)PRCM_BASE;
	dpll_param *ptr = (dpll_param *) get_per_dpll_param();

	/* Moving it to the right sysclk base */
	ptr = ptr + clk_index;

	/* EN_PERIPH_DPLL: CM_CLKEN_PLL[16:18] */
	clrsetbits_le32(&prcm_base->clken_pll, 0x00070000, PLL_STOP << 16);
	wait_on_value(ST_PERIPH_CLK, 0, &prcm_base->idlest_ckgen, LDELAY);

	/*
	 * Errata 1.50 Workaround for OMAP3 ES1.0 only
	 * If using default divisors, write default divisor + 1
	 * and then the actual divisor value
	 */
	/* M6 */
	clrsetbits_le32(&prcm_base->clksel1_emu,
			0x1F000000, (PER_M6X2 + 1) << 24);
	clrsetbits_le32(&prcm_base->clksel1_emu,
			0x1F000000, PER_M6X2 << 24);
	/* M5 */
	clrsetbits_le32(&prcm_base->clksel_cam, 0x0000001F, (PER_M5X2 + 1));
	clrsetbits_le32(&prcm_base->clksel_cam, 0x0000001F, PER_M5X2);
	/* M4 */
	clrsetbits_le32(&prcm_base->clksel_dss, 0x0000001F, (PER_M4X2 + 1));
	clrsetbits_le32(&prcm_base->clksel_dss, 0x0000001F, PER_M4X2);
	/* M3 */
	clrsetbits_le32(&prcm_base->clksel_dss,
			0x00001F00, (PER_M3X2 + 1) << 8);
	clrsetbits_le32(&prcm_base->clksel_dss,
			0x00001F00, PER_M3X2 << 8);
	/* M2 (DIV_96M): CM_CLKSEL3_PLL[0:4] */
	clrsetbits_le32(&prcm_base->clksel3_pll, 0x0000001F, (ptr->m2 + 1));
	clrsetbits_le32(&prcm_base->clksel3_pll, 0x0000001F, ptr->m2);
	/* Workaround end */

	/* M (PERIPH_DPLL_MULT): CM_CLKSEL2_PLL[8:18] */
	clrsetbits_le32(&prcm_base->clksel2_pll,
			0x0007FF00, ptr->m << 8);

	/* N (PERIPH_DPLL_DIV): CM_CLKSEL2_PLL[0:6] */
	clrsetbits_le32(&prcm_base->clksel2_pll, 0x0000007F, ptr->n);

	/* FREQSEL (PERIPH_DPLL_FREQSEL): CM_CLKEN_PLL[20:23] */
	clrsetbits_le32(&prcm_base->clken_pll, 0x00F00000, ptr->fsel << 20);

	/* LOCK MODE (EN_PERIPH_DPLL): CM_CLKEN_PLL[16:18] */
	clrsetbits_le32(&prcm_base->clken_pll, 0x00070000, PLL_LOCK << 16);
	wait_on_value(ST_PERIPH_CLK, 2, &prcm_base->idlest_ckgen, LDELAY);
}

static void dpll5_init_34xx(u32 sil_index, u32 clk_index)
{
	struct prcm *prcm_base = (struct prcm *)PRCM_BASE;
	dpll_param *ptr = (dpll_param *) get_per2_dpll_param();

	/* Moving it to the right sysclk base */
	ptr = ptr + clk_index;

	/* PER2 DPLL (DPLL5) */
	clrsetbits_le32(&prcm_base->clken2_pll, 0x00000007, PLL_STOP);
	wait_on_value(1, 0, &prcm_base->idlest2_ckgen, LDELAY);
	/* set M2 (usbtll_fck) */
	clrsetbits_le32(&prcm_base->clksel5_pll, 0x0000001F, ptr->m2);
	/* set m (11-bit multiplier) */
	clrsetbits_le32(&prcm_base->clksel4_pll, 0x0007FF00, ptr->m << 8);
	/* set n (7-bit divider)*/
	clrsetbits_le32(&prcm_base->clksel4_pll, 0x0000007F, ptr->n);
	/* FREQSEL */
	clrsetbits_le32(&prcm_base->clken_pll, 0x000000F0, ptr->fsel << 4);
	/* lock mode */
	clrsetbits_le32(&prcm_base->clken2_pll, 0x00000007, PLL_LOCK);
	wait_on_value(1, 1, &prcm_base->idlest2_ckgen, LDELAY);
}

static void mpu_init_34xx(u32 sil_index, u32 clk_index)
{
	struct prcm *prcm_base = (struct prcm *)PRCM_BASE;
	dpll_param *ptr = (dpll_param *) get_mpu_dpll_param();

	/* Moving to the right sysclk and ES rev base */
	ptr = ptr + (3 * clk_index) + sil_index;

	/* MPU DPLL (unlocked already) */

	/* M2 (MPU_DPLL_CLKOUT_DIV) : CM_CLKSEL2_PLL_MPU[0:4] */
	clrsetbits_le32(&prcm_base->clksel2_pll_mpu,
			0x0000001F, ptr->m2);

	/* M (MPU_DPLL_MULT) : CM_CLKSEL2_PLL_MPU[8:18] */
	clrsetbits_le32(&prcm_base->clksel1_pll_mpu,
			0x0007FF00, ptr->m << 8);

	/* N (MPU_DPLL_DIV) : CM_CLKSEL2_PLL_MPU[0:6] */
	clrsetbits_le32(&prcm_base->clksel1_pll_mpu,
			0x0000007F, ptr->n);

	/* FREQSEL (MPU_DPLL_FREQSEL) : CM_CLKEN_PLL_MPU[4:7] */
	clrsetbits_le32(&prcm_base->clken_pll_mpu,
			0x000000F0, ptr->fsel << 4);
}

static void iva_init_34xx(u32 sil_index, u32 clk_index)
{
	struct prcm *prcm_base = (struct prcm *)PRCM_BASE;
	dpll_param *ptr = (dpll_param *) get_iva_dpll_param();

	/* Moving to the right sysclk and ES rev base */
	ptr = ptr + (3 * clk_index) + sil_index;

	/* IVA DPLL */
	/* EN_IVA2_DPLL : CM_CLKEN_PLL_IVA2[0:2] */
	clrsetbits_le32(&prcm_base->clken_pll_iva2,
			0x00000007, PLL_STOP);
	wait_on_value(ST_IVA2_CLK, 0, &prcm_base->idlest_pll_iva2, LDELAY);

	/* M2 (IVA2_DPLL_CLKOUT_DIV) : CM_CLKSEL2_PLL_IVA2[0:4] */
	clrsetbits_le32(&prcm_base->clksel2_pll_iva2,
			0x0000001F, ptr->m2);

	/* M (IVA2_DPLL_MULT) : CM_CLKSEL1_PLL_IVA2[8:18] */
	clrsetbits_le32(&prcm_base->clksel1_pll_iva2,
			0x0007FF00, ptr->m << 8);

	/* N (IVA2_DPLL_DIV) : CM_CLKSEL1_PLL_IVA2[0:6] */
	clrsetbits_le32(&prcm_base->clksel1_pll_iva2,
			0x0000007F, ptr->n);

	/* FREQSEL (IVA2_DPLL_FREQSEL) : CM_CLKEN_PLL_IVA2[4:7] */
	clrsetbits_le32(&prcm_base->clken_pll_iva2,
			0x000000F0, ptr->fsel << 4);

	/* LOCK MODE (EN_IVA2_DPLL) : CM_CLKEN_PLL_IVA2[0:2] */
	clrsetbits_le32(&prcm_base->clken_pll_iva2,
			0x00000007, PLL_LOCK);

	wait_on_value(ST_IVA2_CLK, 1, &prcm_base->idlest_pll_iva2, LDELAY);
}

/*
 * OMAP3630 specific functions
 */

static void dpll3_init_36xx(u32 sil_index, u32 clk_index)
{
	struct prcm *prcm_base = (struct prcm *)PRCM_BASE;
	dpll_param *ptr = (dpll_param *) get_36x_core_dpll_param();
	void (*f_lock_pll) (u32, u32, u32, u32);
	int xip_safe, p0, p1, p2, p3;

	xip_safe = is_running_in_sram();

	/* Moving it to the right sysclk base */
	ptr += clk_index;

	if (xip_safe) {
		/* CORE DPLL */

		/* Select relock bypass: CM_CLKEN_PLL[0:2] */
		clrsetbits_le32(&prcm_base->clken_pll,
				0x00000007, PLL_FAST_RELOCK_BYPASS);
		wait_on_value(ST_CORE_CLK, 0, &prcm_base->idlest_ckgen,
				LDELAY);

		/* CM_CLKSEL1_EMU[DIV_DPLL3] */
		clrsetbits_le32(&prcm_base->clksel1_emu,
				0x001F0000, CORE_M3X2 << 16);

		/* M2 (CORE_DPLL_CLKOUT_DIV): CM_CLKSEL1_PLL[27:31] */
		clrsetbits_le32(&prcm_base->clksel1_pll,
				0xF8000000, ptr->m2 << 27);

		/* M (CORE_DPLL_MULT): CM_CLKSEL1_PLL[16:26] */
		clrsetbits_le32(&prcm_base->clksel1_pll,
				0x07FF0000, ptr->m << 16);

		/* N (CORE_DPLL_DIV): CM_CLKSEL1_PLL[8:14] */
		clrsetbits_le32(&prcm_base->clksel1_pll,
				0x00007F00, ptr->n << 8);

		/* Source is the CM_96M_FCLK: CM_CLKSEL1_PLL[6] */
		clrbits_le32(&prcm_base->clksel1_pll, 0x00000040);

		/* SSI */
		clrsetbits_le32(&prcm_base->clksel_core,
				0x00000F00, CORE_SSI_DIV << 8);
		/* FSUSB */
		clrsetbits_le32(&prcm_base->clksel_core,
				0x00000030, CORE_FUSB_DIV << 4);
		/* L4 */
		clrsetbits_le32(&prcm_base->clksel_core,
				0x0000000C, CORE_L4_DIV << 2);
		/* L3 */
		clrsetbits_le32(&prcm_base->clksel_core,
				0x00000003, CORE_L3_DIV);
		/* GFX */
		clrsetbits_le32(&prcm_base->clksel_gfx,
				0x00000007, GFX_DIV_36X);
		/* RESET MGR */
		clrsetbits_le32(&prcm_base->clksel_wkup,
				0x00000006, WKUP_RSM << 1);
		/* FREQSEL (CORE_DPLL_FREQSEL): CM_CLKEN_PLL[4:7] */
		clrsetbits_le32(&prcm_base->clken_pll,
				0x000000F0, ptr->fsel << 4);
		/* LOCK MODE */
		clrsetbits_le32(&prcm_base->clken_pll,
				0x00000007, PLL_LOCK);

		wait_on_value(ST_CORE_CLK, 1, &prcm_base->idlest_ckgen,
				LDELAY);
	} else if (is_running_in_flash()) {
		/*
		 * if running from flash, jump to small relocated code
		 * area in SRAM.
		 */
		f_lock_pll = (void *) (SRAM_CLK_CODE);

		p0 = readl(&prcm_base->clken_pll);
		clrsetbits_le32(&p0, 0x00000007, PLL_FAST_RELOCK_BYPASS);
		/* FREQSEL (CORE_DPLL_FREQSEL): CM_CLKEN_PLL[4:7] */
		clrsetbits_le32(&p0, 0x000000F0, ptr->fsel << 4);

		p1 = readl(&prcm_base->clksel1_pll);
		/* M2 (CORE_DPLL_CLKOUT_DIV): CM_CLKSEL1_PLL[27:31] */
		clrsetbits_le32(&p1, 0xF8000000, ptr->m2 << 27);
		/* M (CORE_DPLL_MULT): CM_CLKSEL1_PLL[16:26] */
		clrsetbits_le32(&p1, 0x07FF0000, ptr->m << 16);
		/* N (CORE_DPLL_DIV): CM_CLKSEL1_PLL[8:14] */
		clrsetbits_le32(&p1, 0x00007F00, ptr->n << 8);
		/* Source is the CM_96M_FCLK: CM_CLKSEL1_PLL[6] */
		clrbits_le32(&p1, 0x00000040);

		p2 = readl(&prcm_base->clksel_core);
		/* SSI */
		clrsetbits_le32(&p2, 0x00000F00, CORE_SSI_DIV << 8);
		/* FSUSB */
		clrsetbits_le32(&p2, 0x00000030, CORE_FUSB_DIV << 4);
		/* L4 */
		clrsetbits_le32(&p2, 0x0000000C, CORE_L4_DIV << 2);
		/* L3 */
		clrsetbits_le32(&p2, 0x00000003, CORE_L3_DIV);

		p3 = (u32)&prcm_base->idlest_ckgen;

		(*f_lock_pll) (p0, p1, p2, p3);
	}
}

static void dpll4_init_36xx(u32 sil_index, u32 clk_index)
{
	struct prcm *prcm_base = (struct prcm *)PRCM_BASE;
	struct dpll_per_36x_param *ptr;

	ptr = (struct dpll_per_36x_param *)get_36x_per_dpll_param();

	/* Moving it to the right sysclk base */
	ptr += clk_index;

	/* EN_PERIPH_DPLL: CM_CLKEN_PLL[16:18] */
	clrsetbits_le32(&prcm_base->clken_pll, 0x00070000, PLL_STOP << 16);
	wait_on_value(ST_PERIPH_CLK, 0, &prcm_base->idlest_ckgen, LDELAY);

	/* M6 (DIV_DPLL4): CM_CLKSEL1_EMU[24:29] */
	clrsetbits_le32(&prcm_base->clksel1_emu, 0x3F000000, ptr->m6 << 24);

	/* M5 (CLKSEL_CAM): CM_CLKSEL1_EMU[0:5] */
	clrsetbits_le32(&prcm_base->clksel_cam, 0x0000003F, ptr->m5);

	/* M4 (CLKSEL_DSS1): CM_CLKSEL_DSS[0:5] */
	clrsetbits_le32(&prcm_base->clksel_dss, 0x0000003F, ptr->m4);

	/* M3 (CLKSEL_DSS1): CM_CLKSEL_DSS[8:13] */
	clrsetbits_le32(&prcm_base->clksel_dss, 0x00003F00, ptr->m3 << 8);

	/* M2 (DIV_96M): CM_CLKSEL3_PLL[0:4] */
	clrsetbits_le32(&prcm_base->clksel3_pll, 0x0000001F, ptr->m2);

	/* M (PERIPH_DPLL_MULT): CM_CLKSEL2_PLL[8:19] */
	clrsetbits_le32(&prcm_base->clksel2_pll, 0x000FFF00, ptr->m << 8);

	/* N (PERIPH_DPLL_DIV): CM_CLKSEL2_PLL[0:6] */
	clrsetbits_le32(&prcm_base->clksel2_pll, 0x0000007F, ptr->n);

	/* M2DIV (CLKSEL_96M): CM_CLKSEL_CORE[12:13] */
	clrsetbits_le32(&prcm_base->clksel_core, 0x00003000, ptr->m2div << 12);

	/* LOCK MODE (EN_PERIPH_DPLL): CM_CLKEN_PLL[16:18] */
	clrsetbits_le32(&prcm_base->clken_pll, 0x00070000, PLL_LOCK << 16);
	wait_on_value(ST_PERIPH_CLK, 2, &prcm_base->idlest_ckgen, LDELAY);
}

static void dpll5_init_36xx(u32 sil_index, u32 clk_index)
{
	struct prcm *prcm_base = (struct prcm *)PRCM_BASE;
	dpll_param *ptr = (dpll_param *) get_36x_per2_dpll_param();

	/* Moving it to the right sysclk base */
	ptr = ptr + clk_index;

	/* PER2 DPLL (DPLL5) */
	clrsetbits_le32(&prcm_base->clken2_pll, 0x00000007, PLL_STOP);
	wait_on_value(1, 0, &prcm_base->idlest2_ckgen, LDELAY);
	/* set M2 (usbtll_fck) */
	clrsetbits_le32(&prcm_base->clksel5_pll, 0x0000001F, ptr->m2);
	/* set m (11-bit multiplier) */
	clrsetbits_le32(&prcm_base->clksel4_pll, 0x0007FF00, ptr->m << 8);
	/* set n (7-bit divider)*/
	clrsetbits_le32(&prcm_base->clksel4_pll, 0x0000007F, ptr->n);
	/* lock mode */
	clrsetbits_le32(&prcm_base->clken2_pll, 0x00000007, PLL_LOCK);
	wait_on_value(1, 1, &prcm_base->idlest2_ckgen, LDELAY);
}

static void mpu_init_36xx(u32 sil_index, u32 clk_index)
{
	struct prcm *prcm_base = (struct prcm *)PRCM_BASE;
	dpll_param *ptr = (dpll_param *) get_36x_mpu_dpll_param();

	/* Moving to the right sysclk */
	ptr += clk_index;

	/* MPU DPLL (unlocked already */

	/* M2 (MPU_DPLL_CLKOUT_DIV) : CM_CLKSEL2_PLL_MPU[0:4] */
	clrsetbits_le32(&prcm_base->clksel2_pll_mpu, 0x0000001F, ptr->m2);

	/* M (MPU_DPLL_MULT) : CM_CLKSEL2_PLL_MPU[8:18] */
	clrsetbits_le32(&prcm_base->clksel1_pll_mpu, 0x0007FF00, ptr->m << 8);

	/* N (MPU_DPLL_DIV) : CM_CLKSEL2_PLL_MPU[0:6] */
	clrsetbits_le32(&prcm_base->clksel1_pll_mpu, 0x0000007F, ptr->n);
}

static void iva_init_36xx(u32 sil_index, u32 clk_index)
{
	struct prcm *prcm_base = (struct prcm *)PRCM_BASE;
	dpll_param *ptr = (dpll_param *)get_36x_iva_dpll_param();

	/* Moving to the right sysclk */
	ptr += clk_index;

	/* IVA DPLL */
	/* EN_IVA2_DPLL : CM_CLKEN_PLL_IVA2[0:2] */
	clrsetbits_le32(&prcm_base->clken_pll_iva2, 0x00000007, PLL_STOP);
	wait_on_value(ST_IVA2_CLK, 0, &prcm_base->idlest_pll_iva2, LDELAY);

	/* M2 (IVA2_DPLL_CLKOUT_DIV) : CM_CLKSEL2_PLL_IVA2[0:4] */
	clrsetbits_le32(&prcm_base->clksel2_pll_iva2, 0x0000001F, ptr->m2);

	/* M (IVA2_DPLL_MULT) : CM_CLKSEL1_PLL_IVA2[8:18] */
	clrsetbits_le32(&prcm_base->clksel1_pll_iva2, 0x0007FF00, ptr->m << 8);

	/* N (IVA2_DPLL_DIV) : CM_CLKSEL1_PLL_IVA2[0:6] */
	clrsetbits_le32(&prcm_base->clksel1_pll_iva2, 0x0000007F, ptr->n);

	/* LOCK (MODE (EN_IVA2_DPLL) : CM_CLKEN_PLL_IVA2[0:2] */
	clrsetbits_le32(&prcm_base->clken_pll_iva2, 0x00000007, PLL_LOCK);

	wait_on_value(ST_IVA2_CLK, 1, &prcm_base->idlest_pll_iva2, LDELAY);
}

/******************************************************************************
 * prcm_init() - inits clocks for PRCM as defined in clocks.h
 *               called from SRAM, or Flash (using temp SRAM stack).
 *****************************************************************************/
void prcm_init(void)
{
	u32 osc_clk = 0, sys_clkin_sel;
	u32 clk_index, sil_index = 0;
	struct prm *prm_base = (struct prm *)PRM_BASE;
	struct prcm *prcm_base = (struct prcm *)PRCM_BASE;

	/*
	 * Gauge the input clock speed and find out the sys_clkin_sel
	 * value corresponding to the input clock.
	 */
	osc_clk = get_osc_clk_speed();
	get_sys_clkin_sel(osc_clk, &sys_clkin_sel);

	/* set input crystal speed */
	clrsetbits_le32(&prm_base->clksel, 0x00000007, sys_clkin_sel);

	/* If the input clock is greater than 19.2M always divide/2 */
	if (sys_clkin_sel > 2) {
		/* input clock divider */
		clrsetbits_le32(&prm_base->clksrc_ctrl, 0x000000C0, 2 << 6);
		clk_index = sys_clkin_sel / 2;
	} else {
		/* input clock divider */
		clrsetbits_le32(&prm_base->clksrc_ctrl, 0x000000C0, 1 << 6);
		clk_index = sys_clkin_sel;
	}

	if (get_cpu_family() == CPU_OMAP36XX) {
		/*
		 * In warm reset conditions on OMAP36xx/AM/DM37xx
		 * the rom code incorrectly sets the DPLL4 clock
		 * input divider to /6.5. Section 3.5.3.3.3.2.1 of
		 * the AM/DM37x TRM explains that the /6.5 divider
		 * is used only when the input clock is 13MHz.
		 *
		 * If the part is in this cpu family *and* the input
		 * clock *is not* 13 MHz, then reset the DPLL4 clock
		 * input divider to /1 as it should never set to /6.5
		 * in this case.
		 */
		if (sys_clkin_sel != 1) {	/* 13 MHz */
			/* Bit 8: DPLL4_CLKINP_DIV */
			clrbits_le32(&prm_base->clksrc_ctrl, 0x00000100);
		}

		/* Unlock MPU DPLL (slows things down, and needed later) */
		clrsetbits_le32(&prcm_base->clken_pll_mpu,
				0x00000007, PLL_LOW_POWER_BYPASS);
		wait_on_value(ST_MPU_CLK, 0, &prcm_base->idlest_pll_mpu,
				LDELAY);

		dpll3_init_36xx(0, clk_index);
		dpll4_init_36xx(0, clk_index);
		dpll5_init_36xx(0, clk_index);
		iva_init_36xx(0, clk_index);
		mpu_init_36xx(0, clk_index);

		/* Lock MPU DPLL to set frequency */
		clrsetbits_le32(&prcm_base->clken_pll_mpu,
				0x00000007, PLL_LOCK);
		wait_on_value(ST_MPU_CLK, 1, &prcm_base->idlest_pll_mpu,
				LDELAY);
	} else {
		/*
		 * The DPLL tables are defined according to sysclk value and
		 * silicon revision. The clk_index value will be used to get
		 * the values for that input sysclk from the DPLL param table
		 * and sil_index will get the values for that SysClk for the
		 * appropriate silicon rev.
		 */
		if (((get_cpu_family() == CPU_OMAP34XX)
				&& (get_cpu_rev() >= CPU_3XX_ES20)) ||
			(get_cpu_family() == CPU_AM35XX))
			sil_index = 1;

		/* Unlock MPU DPLL (slows things down, and needed later) */
		clrsetbits_le32(&prcm_base->clken_pll_mpu,
				0x00000007, PLL_LOW_POWER_BYPASS);
		wait_on_value(ST_MPU_CLK, 0, &prcm_base->idlest_pll_mpu,
				LDELAY);

		dpll3_init_34xx(sil_index, clk_index);
		dpll4_init_34xx(sil_index, clk_index);
		dpll5_init_34xx(sil_index, clk_index);
		if (get_cpu_family() != CPU_AM35XX)
			iva_init_34xx(sil_index, clk_index);

		mpu_init_34xx(sil_index, clk_index);

		/* Lock MPU DPLL to set frequency */
		clrsetbits_le32(&prcm_base->clken_pll_mpu,
				0x00000007, PLL_LOCK);
		wait_on_value(ST_MPU_CLK, 1, &prcm_base->idlest_pll_mpu,
				LDELAY);
	}

	/* Set up GPTimers to sys_clk source only */
	setbits_le32(&prcm_base->clksel_per, 0x000000FF);
	setbits_le32(&prcm_base->clksel_wkup, 1);

	sdelay(5000);
}

/*
 * Enable usb ehci uhh, tll clocks
 */
void ehci_clocks_enable(void)
{
	struct prcm *prcm_base = (struct prcm *)PRCM_BASE;

	/* Enable USBHOST_L3_ICLK (USBHOST_MICLK) */
	setbits_le32(&prcm_base->iclken_usbhost, 1);
	/*
	 * Enable USBHOST_48M_FCLK (USBHOST_FCLK1)
	 * and USBHOST_120M_FCLK (USBHOST_FCLK2)
	 */
	setbits_le32(&prcm_base->fclken_usbhost, 0x00000003);
	/* Enable USBTTL_ICLK */
	setbits_le32(&prcm_base->iclken3_core, 0x00000004);
	/* Enable USBTTL_FCLK */
	setbits_le32(&prcm_base->fclken3_core, 0x00000004);
}

/******************************************************************************
 * peripheral_enable() - Enable the clks & power for perifs (GPT2, UART1,...)
 *****************************************************************************/
void per_clocks_enable(void)
{
	struct prcm *prcm_base = (struct prcm *)PRCM_BASE;

	/* Enable GP2 timer. */
	setbits_le32(&prcm_base->clksel_per, 0x01);	/* GPT2 = sys clk */
	setbits_le32(&prcm_base->iclken_per, 0x08);	/* ICKen GPT2 */
	setbits_le32(&prcm_base->fclken_per, 0x08);	/* FCKen GPT2 */

	/* Enable GP9 timer. */
	setbits_le32(&prcm_base->clksel_per, 0x80);	/* GPT9 = 32kHz clk */
	setbits_le32(&prcm_base->iclken_per, 0x400);	/* ICKen GPT9 */
	setbits_le32(&prcm_base->fclken_per, 0x400);	/* FCKen GPT9 */

#ifdef CONFIG_SYS_NS16550
	/* Enable UART1 clocks */
	setbits_le32(&prcm_base->fclken1_core, 0x00002000);
	setbits_le32(&prcm_base->iclken1_core, 0x00002000);

	/* Enable UART2 clocks */
	setbits_le32(&prcm_base->fclken1_core, 0x00004000);
	setbits_le32(&prcm_base->iclken1_core, 0x00004000);

	/* UART 3 Clocks */
	setbits_le32(&prcm_base->fclken_per, 0x00000800);
	setbits_le32(&prcm_base->iclken_per, 0x00000800);
#endif

#if defined(CONFIG_OMAP3_GPIO_2)
	setbits_le32(&prcm_base->fclken_per, 0x00002000);
	setbits_le32(&prcm_base->iclken_per, 0x00002000);
#endif
#if defined(CONFIG_OMAP3_GPIO_3)
	setbits_le32(&prcm_base->fclken_per, 0x00004000);
	setbits_le32(&prcm_base->iclken_per, 0x00004000);
#endif
#if defined(CONFIG_OMAP3_GPIO_4)
	setbits_le32(&prcm_base->fclken_per, 0x00008000);
	setbits_le32(&prcm_base->iclken_per, 0x00008000);
#endif
#if defined(CONFIG_OMAP3_GPIO_5)
	setbits_le32(&prcm_base->fclken_per, 0x00010000);
	setbits_le32(&prcm_base->iclken_per, 0x00010000);
#endif
#if defined(CONFIG_OMAP3_GPIO_6)
	setbits_le32(&prcm_base->fclken_per, 0x00020000);
	setbits_le32(&prcm_base->iclken_per, 0x00020000);
#endif

#ifdef CONFIG_SYS_I2C_OMAP24XX
	/* Turn on all 3 I2C clocks */
	setbits_le32(&prcm_base->fclken1_core, 0x00038000);
	setbits_le32(&prcm_base->iclken1_core, 0x00038000); /* I2C1,2,3 = on */
#endif
	/* Enable the ICLK for 32K Sync Timer as its used in udelay */
	setbits_le32(&prcm_base->iclken_wkup, 0x00000004);

	if (get_cpu_family() != CPU_AM35XX)
		out_le32(&prcm_base->fclken_iva2, FCK_IVA2_ON);

	out_le32(&prcm_base->fclken1_core, FCK_CORE1_ON);
	out_le32(&prcm_base->iclken1_core, ICK_CORE1_ON);
	out_le32(&prcm_base->iclken2_core, ICK_CORE2_ON);
	out_le32(&prcm_base->fclken_wkup, FCK_WKUP_ON);
	out_le32(&prcm_base->iclken_wkup, ICK_WKUP_ON);
	out_le32(&prcm_base->fclken_dss, FCK_DSS_ON);
	out_le32(&prcm_base->iclken_dss, ICK_DSS_ON);
	if (get_cpu_family() != CPU_AM35XX) {
		out_le32(&prcm_base->fclken_cam, FCK_CAM_ON);
		out_le32(&prcm_base->iclken_cam, ICK_CAM_ON);
	}

	sdelay(1000);
}
