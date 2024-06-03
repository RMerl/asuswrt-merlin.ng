// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2013-2016, Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/clock.h>
#include <asm/arch/mc_cgm_regs.h>
#include <asm/arch/mc_me_regs.h>
#include <asm/arch/mc_rgm_regs.h>
#include <netdev.h>
#include <div64.h>
#include <errno.h>

u32 get_cpu_rev(void)
{
	struct mscm_ir *mscmir = (struct mscm_ir *)MSCM_BASE_ADDR;
	u32 cpu = readl(&mscmir->cpxtype);

	return cpu;
}

DECLARE_GLOBAL_DATA_PTR;

static uintptr_t get_pllfreq(u32 pll, u32 refclk_freq, u32 plldv,
			     u32 pllfd, u32 selected_output)
{
	u32 vco = 0, plldv_prediv = 0, plldv_mfd = 0, pllfd_mfn = 0;
	u32 plldv_rfdphi_div = 0, fout = 0;
	u32 dfs_portn = 0, dfs_mfn = 0, dfs_mfi = 0;

	if (selected_output > DFS_MAXNUMBER) {
		return -1;
	}

	plldv_prediv =
	    (plldv & PLLDIG_PLLDV_PREDIV_MASK) >> PLLDIG_PLLDV_PREDIV_OFFSET;
	plldv_mfd = (plldv & PLLDIG_PLLDV_MFD_MASK);

	pllfd_mfn = (pllfd & PLLDIG_PLLFD_MFN_MASK);

	plldv_prediv = plldv_prediv == 0 ? 1 : plldv_prediv;

	/* The formula for VCO is from TR manual, rev. D */
	vco = refclk_freq / plldv_prediv * (plldv_mfd + pllfd_mfn / 20481);

	if (selected_output != 0) {
		/* Determine the RFDPHI for PHI1 */
		plldv_rfdphi_div =
		    (plldv & PLLDIG_PLLDV_RFDPHI1_MASK) >>
		    PLLDIG_PLLDV_RFDPHI1_OFFSET;
		plldv_rfdphi_div = plldv_rfdphi_div == 0 ? 1 : plldv_rfdphi_div;
		if (pll == ARM_PLL || pll == ENET_PLL || pll == DDR_PLL) {
			dfs_portn =
			    readl(DFS_DVPORTn(pll, selected_output - 1));
			dfs_mfi =
			    (dfs_portn & DFS_DVPORTn_MFI_MASK) >>
			    DFS_DVPORTn_MFI_OFFSET;
			dfs_mfn =
			    (dfs_portn & DFS_DVPORTn_MFI_MASK) >>
			    DFS_DVPORTn_MFI_OFFSET;
			fout = vco / (dfs_mfi + (dfs_mfn / 256));
		} else {
			fout = vco / plldv_rfdphi_div;
		}

	} else {
		/* Determine the RFDPHI for PHI0 */
		plldv_rfdphi_div =
		    (plldv & PLLDIG_PLLDV_RFDPHI_MASK) >>
		    PLLDIG_PLLDV_RFDPHI_OFFSET;
		fout = vco / plldv_rfdphi_div;
	}

	return fout;

}

/* Implemented for ARMPLL, PERIPH_PLL, ENET_PLL, DDR_PLL, VIDEO_LL */
static uintptr_t decode_pll(enum pll_type pll, u32 refclk_freq,
			    u32 selected_output)
{
	u32 plldv, pllfd;

	plldv = readl(PLLDIG_PLLDV(pll));
	pllfd = readl(PLLDIG_PLLFD(pll));

	return get_pllfreq(pll, refclk_freq, plldv, pllfd, selected_output);
}

static u32 get_mcu_main_clk(void)
{
	u32 coreclk_div;
	u32 sysclk_sel;
	u32 freq = 0;

	sysclk_sel = readl(CGM_SC_SS(MC_CGM1_BASE_ADDR)) & MC_CGM_SC_SEL_MASK;
	sysclk_sel >>= MC_CGM_SC_SEL_OFFSET;

	coreclk_div =
	    readl(CGM_SC_DCn(MC_CGM1_BASE_ADDR, 0)) & MC_CGM_SC_DCn_PREDIV_MASK;
	coreclk_div >>= MC_CGM_SC_DCn_PREDIV_OFFSET;
	coreclk_div += 1;

	switch (sysclk_sel) {
	case MC_CGM_SC_SEL_FIRC:
		freq = FIRC_CLK_FREQ;
		break;
	case MC_CGM_SC_SEL_XOSC:
		freq = XOSC_CLK_FREQ;
		break;
	case MC_CGM_SC_SEL_ARMPLL:
		/* ARMPLL has as source XOSC and CORE_CLK has as input PHI0 */
		freq = decode_pll(ARM_PLL, XOSC_CLK_FREQ, 0);
		break;
	case MC_CGM_SC_SEL_CLKDISABLE:
		printf("Sysclk is disabled\n");
		break;
	default:
		printf("unsupported system clock select\n");
	}

	return freq / coreclk_div;
}

static u32 get_sys_clk(u32 number)
{
	u32 sysclk_div, sysclk_div_number;
	u32 sysclk_sel;
	u32 freq = 0;

	switch (number) {
	case 3:
		sysclk_div_number = 0;
		break;
	case 6:
		sysclk_div_number = 1;
		break;
	default:
		printf("unsupported system clock \n");
		return -1;
	}
	sysclk_sel = readl(CGM_SC_SS(MC_CGM0_BASE_ADDR)) & MC_CGM_SC_SEL_MASK;
	sysclk_sel >>= MC_CGM_SC_SEL_OFFSET;

	sysclk_div =
	    readl(CGM_SC_DCn(MC_CGM1_BASE_ADDR, sysclk_div_number)) &
	    MC_CGM_SC_DCn_PREDIV_MASK;
	sysclk_div >>= MC_CGM_SC_DCn_PREDIV_OFFSET;
	sysclk_div += 1;

	switch (sysclk_sel) {
	case MC_CGM_SC_SEL_FIRC:
		freq = FIRC_CLK_FREQ;
		break;
	case MC_CGM_SC_SEL_XOSC:
		freq = XOSC_CLK_FREQ;
		break;
	case MC_CGM_SC_SEL_ARMPLL:
		/* ARMPLL has as source XOSC and SYSn_CLK has as input DFS1 */
		freq = decode_pll(ARM_PLL, XOSC_CLK_FREQ, 1);
		break;
	case MC_CGM_SC_SEL_CLKDISABLE:
		printf("Sysclk is disabled\n");
		break;
	default:
		printf("unsupported system clock select\n");
	}

	return freq / sysclk_div;
}

static u32 get_peripherals_clk(void)
{
	u32 aux5clk_div;
	u32 freq = 0;

	aux5clk_div =
	    readl(CGM_ACn_DCm(MC_CGM0_BASE_ADDR, 5, 0)) &
	    MC_CGM_ACn_DCm_PREDIV_MASK;
	aux5clk_div >>= MC_CGM_ACn_DCm_PREDIV_OFFSET;
	aux5clk_div += 1;

	freq = decode_pll(PERIPH_PLL, XOSC_CLK_FREQ, 0);

	return freq / aux5clk_div;

}

static u32 get_uart_clk(void)
{
	u32 auxclk3_div, auxclk3_sel, freq = 0;

	auxclk3_sel =
	    readl(CGM_ACn_SS(MC_CGM0_BASE_ADDR, 3)) & MC_CGM_ACn_SEL_MASK;
	auxclk3_sel >>= MC_CGM_ACn_SEL_OFFSET;

	auxclk3_div =
	    readl(CGM_ACn_DCm(MC_CGM0_BASE_ADDR, 3, 0)) &
	    MC_CGM_ACn_DCm_PREDIV_MASK;
	auxclk3_div >>= MC_CGM_ACn_DCm_PREDIV_OFFSET;
	auxclk3_div += 1;

	switch (auxclk3_sel) {
	case MC_CGM_ACn_SEL_FIRC:
		freq = FIRC_CLK_FREQ;
		break;
	case MC_CGM_ACn_SEL_XOSC:
		freq = XOSC_CLK_FREQ;
		break;
	case MC_CGM_ACn_SEL_PERPLLDIVX:
		freq = get_peripherals_clk() / 3;
		break;
	case MC_CGM_ACn_SEL_SYSCLK:
		freq = get_sys_clk(6);
		break;
	default:
		printf("unsupported system clock select\n");
	}

	return freq / auxclk3_div;
}

static u32 get_fec_clk(void)
{
	u32 aux2clk_div;
	u32 freq = 0;

	aux2clk_div =
	    readl(CGM_ACn_DCm(MC_CGM0_BASE_ADDR, 2, 0)) &
	    MC_CGM_ACn_DCm_PREDIV_MASK;
	aux2clk_div >>= MC_CGM_ACn_DCm_PREDIV_OFFSET;
	aux2clk_div += 1;

	freq = decode_pll(ENET_PLL, XOSC_CLK_FREQ, 0);

	return freq / aux2clk_div;
}

static u32 get_usdhc_clk(void)
{
	u32 aux15clk_div;
	u32 freq = 0;

	aux15clk_div =
	    readl(CGM_ACn_DCm(MC_CGM0_BASE_ADDR, 15, 0)) &
	    MC_CGM_ACn_DCm_PREDIV_MASK;
	aux15clk_div >>= MC_CGM_ACn_DCm_PREDIV_OFFSET;
	aux15clk_div += 1;

	freq = decode_pll(ENET_PLL, XOSC_CLK_FREQ, 4);

	return freq / aux15clk_div;
}

static u32 get_i2c_clk(void)
{
	return get_peripherals_clk();
}

/* return clocks in Hz */
unsigned int mxc_get_clock(enum mxc_clock clk)
{
	switch (clk) {
	case MXC_ARM_CLK:
		return get_mcu_main_clk();
	case MXC_PERIPHERALS_CLK:
		return get_peripherals_clk();
	case MXC_UART_CLK:
		return get_uart_clk();
	case MXC_FEC_CLK:
		return get_fec_clk();
	case MXC_I2C_CLK:
		return get_i2c_clk();
	case MXC_USDHC_CLK:
		return get_usdhc_clk();
	default:
		break;
	}
	printf("Error: Unsupported function to read the frequency! \
			Please define it correctly!");
	return -1;
}

/* Not yet implemented - int soc_clk_dump(); */

#if defined(CONFIG_DISPLAY_CPUINFO)
static char *get_reset_cause(void)
{
	u32 cause = readl(MC_RGM_BASE_ADDR + 0x300);

	switch (cause) {
	case F_SWT4:
		return "WDOG";
	case F_JTAG:
		return "JTAG";
	case F_FCCU_SOFT:
		return "FCCU soft reaction";
	case F_FCCU_HARD:
		return "FCCU hard reaction";
	case F_SOFT_FUNC:
		return "Software Functional reset";
	case F_ST_DONE:
		return "Self Test done reset";
	case F_EXT_RST:
		return "External reset";
	default:
		return "unknown reset";
	}

}

#define SRC_SCR_SW_RST					(1<<12)

void reset_cpu(ulong addr)
{
	printf("Feature not supported.\n");
};

int print_cpuinfo(void)
{
	printf("CPU:   Freescale Treerunner S32V234 at %d MHz\n",
	       mxc_get_clock(MXC_ARM_CLK) / 1000000);
	printf("Reset cause: %s\n", get_reset_cause());

	return 0;
}
#endif

int cpu_eth_init(bd_t * bis)
{
	int rc = -ENODEV;

#if defined(CONFIG_FEC_MXC)
	rc = fecmxc_initialize(bis);
#endif

	return rc;
}

int get_clocks(void)
{
#ifdef CONFIG_FSL_ESDHC
	gd->arch.sdhc_clk = mxc_get_clock(MXC_USDHC_CLK);
#endif
	return 0;
}
