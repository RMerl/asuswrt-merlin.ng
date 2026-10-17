// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2013 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <clock_legacy.h>
#include <command.h>
#include <cpu_func.h>
#include <init.h>
#include <net.h>
#include <asm/cache.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/clock.h>
#include <asm/arch/crm_regs.h>
#include <asm/mach-imx/sys_proto.h>
#include <env.h>
#include <netdev.h>
#ifdef CONFIG_FSL_ESDHC_IMX
#include <fsl_esdhc_imx.h>
#endif

#ifdef CONFIG_FSL_ESDHC_IMX
DECLARE_GLOBAL_DATA_PTR;
#endif

static char soc_type[] = "xx0";

#ifdef CONFIG_MXC_OCOTP
void enable_ocotp_clk(unsigned char enable)
{
	struct ccm_reg *ccm = (struct ccm_reg *)CCM_BASE_ADDR;
	u32 reg;

	reg = readl(&ccm->ccgr6);
	if (enable)
		reg |= CCM_CCGR6_OCOTP_CTRL_MASK;
	else
		reg &= ~CCM_CCGR6_OCOTP_CTRL_MASK;
	writel(reg, &ccm->ccgr6);
}
#endif

static u32 get_mcu_main_clk(void)
{
	struct ccm_reg *ccm = (struct ccm_reg *)CCM_BASE_ADDR;
	u32 ccm_ccsr, ccm_cacrr, armclk_div;
	u32 sysclk_sel, pll_pfd_sel = 0;
	u32 freq = 0;

	ccm_ccsr = readl(&ccm->ccsr);
	sysclk_sel = ccm_ccsr & CCM_CCSR_SYS_CLK_SEL_MASK;
	sysclk_sel >>= CCM_CCSR_SYS_CLK_SEL_OFFSET;

	ccm_cacrr = readl(&ccm->cacrr);
	armclk_div = ccm_cacrr & CCM_CACRR_ARM_CLK_DIV_MASK;
	armclk_div >>= CCM_CACRR_ARM_CLK_DIV_OFFSET;
	armclk_div += 1;

	switch (sysclk_sel) {
	case 0:
		freq = FASE_CLK_FREQ;
		break;
	case 1:
		freq = SLOW_CLK_FREQ;
		break;
	case 2:
		pll_pfd_sel = ccm_ccsr & CCM_CCSR_PLL2_PFD_CLK_SEL_MASK;
		pll_pfd_sel >>= CCM_CCSR_PLL2_PFD_CLK_SEL_OFFSET;
		if (pll_pfd_sel == 0)
			freq = PLL2_MAIN_FREQ;
		else if (pll_pfd_sel == 1)
			freq = PLL2_PFD1_FREQ;
		else if (pll_pfd_sel == 2)
			freq = PLL2_PFD2_FREQ;
		else if (pll_pfd_sel == 3)
			freq = PLL2_PFD3_FREQ;
		else if (pll_pfd_sel == 4)
			freq = PLL2_PFD4_FREQ;
		break;
	case 3:
		freq = PLL2_MAIN_FREQ;
		break;
	case 4:
		pll_pfd_sel = ccm_ccsr & CCM_CCSR_PLL1_PFD_CLK_SEL_MASK;
		pll_pfd_sel >>= CCM_CCSR_PLL1_PFD_CLK_SEL_OFFSET;
		if (pll_pfd_sel == 0)
			freq = PLL1_MAIN_FREQ;
		else if (pll_pfd_sel == 1)
			freq = PLL1_PFD1_FREQ;
		else if (pll_pfd_sel == 2)
			freq = PLL1_PFD2_FREQ;
		else if (pll_pfd_sel == 3)
			freq = PLL1_PFD3_FREQ;
		else if (pll_pfd_sel == 4)
			freq = PLL1_PFD4_FREQ;
		break;
	case 5:
		freq = PLL3_MAIN_FREQ;
		break;
	default:
		printf("unsupported system clock select\n");
	}

	return freq / armclk_div;
}

static u32 get_bus_clk(void)
{
	struct ccm_reg *ccm = (struct ccm_reg *)CCM_BASE_ADDR;
	u32 ccm_cacrr, busclk_div;

	ccm_cacrr = readl(&ccm->cacrr);

	busclk_div = ccm_cacrr & CCM_CACRR_BUS_CLK_DIV_MASK;
	busclk_div >>= CCM_CACRR_BUS_CLK_DIV_OFFSET;
	busclk_div += 1;

	return get_mcu_main_clk() / busclk_div;
}

static u32 get_ipg_clk(void)
{
	struct ccm_reg *ccm = (struct ccm_reg *)CCM_BASE_ADDR;
	u32 ccm_cacrr, ipgclk_div;

	ccm_cacrr = readl(&ccm->cacrr);

	ipgclk_div = ccm_cacrr & CCM_CACRR_IPG_CLK_DIV_MASK;
	ipgclk_div >>= CCM_CACRR_IPG_CLK_DIV_OFFSET;
	ipgclk_div += 1;

	return get_bus_clk() / ipgclk_div;
}

static u32 get_uart_clk(void)
{
	return get_ipg_clk();
}

static u32 get_sdhc_clk(void)
{
	struct ccm_reg *ccm = (struct ccm_reg *)CCM_BASE_ADDR;
	u32 ccm_cscmr1, ccm_cscdr2, sdhc_clk_sel, sdhc_clk_div;
	u32 freq = 0;

	ccm_cscmr1 = readl(&ccm->cscmr1);
	sdhc_clk_sel = ccm_cscmr1 & CCM_CSCMR1_ESDHC1_CLK_SEL_MASK;
	sdhc_clk_sel >>= CCM_CSCMR1_ESDHC1_CLK_SEL_OFFSET;

	ccm_cscdr2 = readl(&ccm->cscdr2);
	sdhc_clk_div = ccm_cscdr2 & CCM_CSCDR2_ESDHC1_CLK_DIV_MASK;
	sdhc_clk_div >>= CCM_CSCDR2_ESDHC1_CLK_DIV_OFFSET;
	sdhc_clk_div += 1;

	switch (sdhc_clk_sel) {
	case 0:
		freq = PLL3_MAIN_FREQ;
		break;
	case 1:
		freq = PLL3_PFD3_FREQ;
		break;
	case 2:
		freq = PLL1_PFD3_FREQ;
		break;
	case 3:
		freq = get_bus_clk();
		break;
	}

	return freq / sdhc_clk_div;
}

u32 get_fec_clk(void)
{
	struct ccm_reg *ccm = (struct ccm_reg *)CCM_BASE_ADDR;
	u32 ccm_cscmr2, rmii_clk_sel;
	u32 freq = 0;

	ccm_cscmr2 = readl(&ccm->cscmr2);
	rmii_clk_sel = ccm_cscmr2 & CCM_CSCMR2_RMII_CLK_SEL_MASK;
	rmii_clk_sel >>= CCM_CSCMR2_RMII_CLK_SEL_OFFSET;

	switch (rmii_clk_sel) {
	case 0:
		freq = ENET_EXTERNAL_CLK;
		break;
	case 1:
		freq = AUDIO_EXTERNAL_CLK;
		break;
	case 2:
		freq = PLL5_MAIN_FREQ;
		break;
	case 3:
		freq = PLL5_MAIN_FREQ / 2;
		break;
	}

	return freq;
}

static u32 get_i2c_clk(void)
{
	return get_ipg_clk();
}

static u32 get_dspi_clk(void)
{
	return get_ipg_clk();
}

u32 get_lpuart_clk(void)
{
	return get_uart_clk();
}

unsigned int mxc_get_clock(enum mxc_clock clk)
{
	switch (clk) {
	case MXC_ARM_CLK:
		return get_mcu_main_clk();
	case MXC_BUS_CLK:
		return get_bus_clk();
	case MXC_IPG_CLK:
		return get_ipg_clk();
	case MXC_UART_CLK:
		return get_uart_clk();
	case MXC_ESDHC_CLK:
		return get_sdhc_clk();
	case MXC_FEC_CLK:
		return get_fec_clk();
	case MXC_I2C_CLK:
		return get_i2c_clk();
	case MXC_DSPI_CLK:
		return get_dspi_clk();
	default:
		break;
	}
	return -1;
}

/* Dump some core clocks */
int do_vf610_showclocks(struct cmd_tbl *cmdtp, int flag, int argc,
			char *const argv[])
{
	printf("\n");
	printf("cpu clock : %8d MHz\n", mxc_get_clock(MXC_ARM_CLK) / 1000000);
	printf("bus clock : %8d MHz\n", mxc_get_clock(MXC_BUS_CLK) / 1000000);
	printf("ipg clock : %8d MHz\n", mxc_get_clock(MXC_IPG_CLK) / 1000000);

	return 0;
}

U_BOOT_CMD(
	clocks, CONFIG_SYS_MAXARGS, 1, do_vf610_showclocks,
	"display clocks",
	""
);

#ifdef CONFIG_FEC_MXC
__weak void imx_get_mac_from_fuse(int dev_id, unsigned char *mac)
{
	struct ocotp_regs *ocotp = (struct ocotp_regs *)OCOTP_BASE_ADDR;
	struct fuse_bank *bank = &ocotp->bank[4];
	struct fuse_bank4_regs *fuse =
		(struct fuse_bank4_regs *)bank->fuse_regs;

	u32 value = readl(&fuse->mac_addr0);
	mac[0] = (value >> 8);
	mac[1] = value;

	value = readl(&fuse->mac_addr1);
	mac[2] = value >> 24;
	mac[3] = value >> 16;
	mac[4] = value >> 8;
	mac[5] = value;
}
#endif

u32 get_cpu_rev(void)
{
	return MXC_CPU_VF610 << 12;
}

#if defined(CONFIG_DISPLAY_CPUINFO)
static char *get_reset_cause(void)
{
	u32 cause;
	struct src *src_regs = (struct src *)SRC_BASE_ADDR;

	cause = readl(&src_regs->srsr);
	writel(cause, &src_regs->srsr);

	if (cause & SRC_SRSR_POR_RST)
		return "POWER ON RESET";
	else if (cause & SRC_SRSR_WDOG_A5)
		return "WDOG A5";
	else if (cause & SRC_SRSR_WDOG_M4)
		return "WDOG M4";
	else if (cause & SRC_SRSR_JTAG_RST)
		return "JTAG HIGH-Z";
	else if (cause & SRC_SRSR_SW_RST)
		return "SW RESET";
	else if (cause & SRC_SRSR_RESETB)
		return "EXTERNAL RESET";
	else
		return "unknown reset";
}

int print_cpuinfo(void)
{
	printf("CPU: Freescale Vybrid VF%s at %d MHz\n",
	       soc_type, mxc_get_clock(MXC_ARM_CLK) / 1000000);
	printf("Reset cause: %s\n", get_reset_cause());

	return 0;
}
#endif

int arch_cpu_init(void)
{
	struct mscm *mscm = (struct mscm *)MSCM_BASE_ADDR;

	soc_type[0] = mscm->cpxcount ? '6' : '5'; /*Dual Core => VF6x0 */
	soc_type[1] = mscm->cpxcfg1 ? '1' : '0'; /* L2 Cache => VFx10 */

	return 0;
}

#ifdef CONFIG_ARCH_MISC_INIT
int arch_misc_init(void)
{
	char soc[6];

	strcpy(soc, "vf");
	strcat(soc, soc_type);
	env_set("soc", soc);

	return 0;
}
#endif

int cpu_eth_init(struct bd_info *bis)
{
	int rc = -ENODEV;

#if defined(CONFIG_FEC_MXC)
	rc = fecmxc_initialize(bis);
#endif

	return rc;
}

#ifdef CONFIG_FSL_ESDHC_IMX
int cpu_mmc_init(struct bd_info *bis)
{
	return fsl_esdhc_mmc_init(bis);
}
#endif

int get_clocks(void)
{
#ifdef CONFIG_FSL_ESDHC_IMX
	gd->arch.sdhc_clk = mxc_get_clock(MXC_ESDHC_CLK);
#endif
	return 0;
}

#if !CONFIG_IS_ENABLED(SYS_DCACHE_OFF)
void enable_caches(void)
{
#if defined(CONFIG_SYS_ARM_CACHE_WRITETHROUGH)
	enum dcache_option option = DCACHE_WRITETHROUGH;
#else
	enum dcache_option option = DCACHE_WRITEBACK;
#endif
	dcache_enable();
	icache_enable();

    /* Enable caching on OCRAM */
	mmu_set_region_dcache_behaviour(IRAM_BASE_ADDR, IRAM_SIZE, option);
}
#endif

#ifdef CONFIG_SYS_I2C_MXC
/* i2c_num can be from 0 - 3 */
int enable_i2c_clk(unsigned char enable, unsigned int i2c_num)
{
	struct ccm_reg *ccm = (struct ccm_reg *)CCM_BASE_ADDR;

	switch (i2c_num) {
	case 0:
		clrsetbits_le32(&ccm->ccgr4, CCM_CCGR4_I2C0_CTRL_MASK,
				CCM_CCGR4_I2C0_CTRL_MASK);
	case 2:
		clrsetbits_le32(&ccm->ccgr10, CCM_CCGR10_I2C2_CTRL_MASK,
				CCM_CCGR10_I2C2_CTRL_MASK);
		break;
	default:
		return -EINVAL;
	}

	return 0;
}
#endif
