// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2007
 * Sascha Hauer, Pengutronix
 *
 * (C) Copyright 2008-2010 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <div64.h>
#include <asm/io.h>
#include <linux/errno.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#ifdef CONFIG_FSL_ESDHC
#include <fsl_esdhc.h>
#endif
#include <netdev.h>
#include <spl.h>

#define CLK_CODE(arm, ahb, sel) (((arm) << 16) + ((ahb) << 8) + (sel))
#define CLK_CODE_ARM(c)		(((c) >> 16) & 0xFF)
#define CLK_CODE_AHB(c)		(((c) >>  8) & 0xFF)
#define CLK_CODE_PATH(c)	((c) & 0xFF)

#define CCM_GET_DIVIDER(x, m, o) (((x) & (m)) >> (o))

#ifdef CONFIG_FSL_ESDHC
DECLARE_GLOBAL_DATA_PTR;
#endif

static int g_clk_mux_auto[8] = {
	CLK_CODE(1, 3, 0), CLK_CODE(1, 2, 1), CLK_CODE(2, 1, 1), -1,
	CLK_CODE(1, 6, 0), CLK_CODE(1, 4, 1), CLK_CODE(2, 2, 1), -1,
};

static int g_clk_mux_consumer[16] = {
	CLK_CODE(1, 4, 0), CLK_CODE(1, 3, 1), CLK_CODE(1, 3, 1), -1,
	-1, -1, CLK_CODE(4, 1, 0), CLK_CODE(1, 5, 0),
	CLK_CODE(1, 8, 1), CLK_CODE(1, 6, 1), CLK_CODE(2, 4, 0), -1,
	-1, -1, CLK_CODE(4, 2, 0), -1,
};

static int hsp_div_table[3][16] = {
	{4, 3, 2, -1, -1, -1, 1, 5, 4, 3, 2, -1, -1, -1, 1, -1},
	{-1, -1, -1, -1, -1, -1, -1, -1, 8, 6, 4, -1, -1, -1, 2, -1},
	{3, -1, -1, -1, -1, -1, -1, -1, 3, -1, -1, -1, -1, -1, -1, -1},
};

u32 get_cpu_rev(void)
{
	int reg;
	struct iim_regs *iim =
		(struct iim_regs *)IIM_BASE_ADDR;
	reg = readl(&iim->iim_srev);
	if (!reg) {
		reg = readw(ROMPATCH_REV);
		reg <<= 4;
	} else {
		reg += CHIP_REV_1_0;
	}

	return 0x35000 + (reg & 0xFF);
}

static u32 get_arm_div(u32 pdr0, u32 *fi, u32 *fd)
{
	int *pclk_mux;
	if (pdr0 & MXC_CCM_PDR0_AUTO_CON) {
		pclk_mux = g_clk_mux_consumer +
			((pdr0 & MXC_CCM_PDR0_CON_MUX_DIV_MASK) >>
			MXC_CCM_PDR0_CON_MUX_DIV_OFFSET);
	} else {
		pclk_mux = g_clk_mux_auto +
			((pdr0 & MXC_CCM_PDR0_AUTO_MUX_DIV_MASK) >>
			MXC_CCM_PDR0_AUTO_MUX_DIV_OFFSET);
	}

	if ((*pclk_mux) == -1)
		return -1;

	if (fi && fd) {
		if (!CLK_CODE_PATH(*pclk_mux)) {
			*fi = *fd = 1;
			return CLK_CODE_ARM(*pclk_mux);
		}
		if (pdr0 & MXC_CCM_PDR0_AUTO_CON) {
			*fi = 3;
			*fd = 4;
		} else {
			*fi = 2;
			*fd = 3;
		}
	}
	return CLK_CODE_ARM(*pclk_mux);
}

static int get_ahb_div(u32 pdr0)
{
	int *pclk_mux;

	pclk_mux = g_clk_mux_consumer +
		((pdr0 & MXC_CCM_PDR0_CON_MUX_DIV_MASK) >>
		MXC_CCM_PDR0_CON_MUX_DIV_OFFSET);

	if ((*pclk_mux) == -1)
		return -1;

	return CLK_CODE_AHB(*pclk_mux);
}

static u32 decode_pll(u32 reg, u32 infreq)
{
	u32 mfi = (reg >> 10) & 0xf;
	s32 mfn = reg & 0x3ff;
	u32 mfd = (reg >> 16) & 0x3ff;
	u32 pd = (reg >> 26) & 0xf;

	mfi = mfi <= 5 ? 5 : mfi;
	mfn = mfn >= 512 ? mfn - 1024 : mfn;
	mfd += 1;
	pd += 1;

	return lldiv(2 * (u64)infreq * (mfi * mfd + mfn),
		mfd * pd);
}

static u32 get_mcu_main_clk(void)
{
	u32 arm_div = 0, fi = 0, fd = 0;
	struct ccm_regs *ccm =
		(struct ccm_regs *)IMX_CCM_BASE;
	arm_div = get_arm_div(readl(&ccm->pdr0), &fi, &fd);
	fi *= decode_pll(readl(&ccm->mpctl), MXC_HCLK);
	return fi / (arm_div * fd);
}

static u32 get_ipg_clk(void)
{
	u32 freq = get_mcu_main_clk();
	struct ccm_regs *ccm =
		(struct ccm_regs *)IMX_CCM_BASE;
	u32 pdr0 = readl(&ccm->pdr0);

	return freq / (get_ahb_div(pdr0) * 2);
}

static u32 get_ipg_per_clk(void)
{
	u32 freq = get_mcu_main_clk();
	struct ccm_regs *ccm =
		(struct ccm_regs *)IMX_CCM_BASE;
	u32 pdr0 = readl(&ccm->pdr0);
	u32 pdr4 = readl(&ccm->pdr4);
	u32 div;
	if (pdr0 & MXC_CCM_PDR0_PER_SEL) {
		div = CCM_GET_DIVIDER(pdr4,
			MXC_CCM_PDR4_PER0_PODF_MASK,
			MXC_CCM_PDR4_PER0_PODF_OFFSET) + 1;
	} else {
		div = CCM_GET_DIVIDER(pdr0,
			MXC_CCM_PDR0_PER_PODF_MASK,
			MXC_CCM_PDR0_PER_PODF_OFFSET) + 1;
		div *= get_ahb_div(pdr0);
	}
	return freq / div;
}

u32 imx_get_uartclk(void)
{
	u32 freq;
	struct ccm_regs *ccm =
		(struct ccm_regs *)IMX_CCM_BASE;
	u32 pdr4 = readl(&ccm->pdr4);

	if (readl(&ccm->pdr3) & MXC_CCM_PDR3_UART_M_U)
		freq = get_mcu_main_clk();
	else
		freq = decode_pll(readl(&ccm->ppctl), MXC_HCLK);
	freq /= CCM_GET_DIVIDER(pdr4,
			MXC_CCM_PDR4_UART_PODF_MASK,
			MXC_CCM_PDR4_UART_PODF_OFFSET) + 1;
	return freq;
}

unsigned int mxc_get_main_clock(enum mxc_main_clock clk)
{
	u32 nfc_pdf, hsp_podf;
	u32 pll, ret_val = 0, usb_podf;
	struct ccm_regs *ccm =
		(struct ccm_regs *)IMX_CCM_BASE;

	u32 reg = readl(&ccm->pdr0);
	u32 reg4 = readl(&ccm->pdr4);

	reg |= 0x1;

	switch (clk) {
	case CPU_CLK:
		ret_val = get_mcu_main_clk();
		break;
	case AHB_CLK:
		ret_val = get_mcu_main_clk();
		break;
	case HSP_CLK:
		if (reg & CLKMODE_CONSUMER) {
			hsp_podf = (reg >> 20) & 0x3;
			pll = get_mcu_main_clk();
			hsp_podf = hsp_div_table[hsp_podf][(reg>>16)&0xF];
			if (hsp_podf > 0) {
				ret_val = pll / hsp_podf;
			} else {
				puts("mismatch HSP with ARM clock setting\n");
				ret_val = 0;
			}
		} else {
			ret_val = get_mcu_main_clk();
		}
		break;
	case IPG_CLK:
		ret_val = get_ipg_clk();
		break;
	case IPG_PER_CLK:
		ret_val = get_ipg_per_clk();
		break;
	case NFC_CLK:
		nfc_pdf = (reg4 >> 28) & 0xF;
		pll = get_mcu_main_clk();
		/* AHB/nfc_pdf */
		ret_val = pll / (nfc_pdf + 1);
		break;
	case USB_CLK:
		usb_podf = (reg4 >> 22) & 0x3F;
		if (reg4 & 0x200)
			pll = get_mcu_main_clk();
		else
			pll = decode_pll(readl(&ccm->ppctl), MXC_HCLK);

		ret_val = pll / (usb_podf + 1);
		break;
	default:
		printf("Unknown clock: %d\n", clk);
		break;
	}

	return ret_val;
}
unsigned int mxc_get_peri_clock(enum mxc_peri_clock clk)
{
	u32 ret_val = 0, pdf, pre_pdf, clk_sel;
	struct ccm_regs *ccm =
		(struct ccm_regs *)IMX_CCM_BASE;
	u32 mpdr2 = readl(&ccm->pdr2);
	u32 mpdr3 = readl(&ccm->pdr3);
	u32 mpdr4 = readl(&ccm->pdr4);

	switch (clk) {
	case UART1_BAUD:
	case UART2_BAUD:
	case UART3_BAUD:
		clk_sel = mpdr3 & (1 << 14);
		pdf = (mpdr4 >> 10) & 0x3F;
		ret_val = ((clk_sel != 0) ? mxc_get_main_clock(CPU_CLK) :
			decode_pll(readl(&ccm->ppctl), MXC_HCLK)) / (pdf + 1);
		break;
	case SSI1_BAUD:
		pre_pdf = (mpdr2 >> 24) & 0x7;
		pdf = mpdr2 & 0x3F;
		clk_sel = mpdr2 & (1 << 6);
		ret_val = ((clk_sel != 0) ? mxc_get_main_clock(CPU_CLK) :
			decode_pll(readl(&ccm->ppctl), MXC_HCLK)) /
				((pre_pdf + 1) * (pdf + 1));
		break;
	case SSI2_BAUD:
		pre_pdf = (mpdr2 >> 27) & 0x7;
		pdf = (mpdr2 >> 8) & 0x3F;
		clk_sel = mpdr2 & (1 << 6);
		ret_val = ((clk_sel != 0) ? mxc_get_main_clock(CPU_CLK) :
			decode_pll(readl(&ccm->ppctl), MXC_HCLK)) /
				((pre_pdf + 1) * (pdf + 1));
		break;
	case CSI_BAUD:
		clk_sel = mpdr2 & (1 << 7);
		pdf = (mpdr2 >> 16) & 0x3F;
		ret_val = ((clk_sel != 0) ? mxc_get_main_clock(CPU_CLK) :
			decode_pll(readl(&ccm->ppctl), MXC_HCLK)) / (pdf + 1);
		break;
	case MSHC_CLK:
		pre_pdf = readl(&ccm->pdr1);
		clk_sel = (pre_pdf & 0x80);
		pdf = (pre_pdf >> 22) & 0x3F;
		pre_pdf = (pre_pdf >> 28) & 0x7;
		ret_val = ((clk_sel != 0) ? mxc_get_main_clock(CPU_CLK) :
			decode_pll(readl(&ccm->ppctl), MXC_HCLK)) /
				((pre_pdf + 1) * (pdf + 1));
		break;
	case ESDHC1_CLK:
		clk_sel = mpdr3 & 0x40;
		pdf = mpdr3 & 0x3F;
		ret_val = ((clk_sel != 0) ? mxc_get_main_clock(CPU_CLK) :
			decode_pll(readl(&ccm->ppctl), MXC_HCLK)) / (pdf + 1);
		break;
	case ESDHC2_CLK:
		clk_sel = mpdr3 & 0x40;
		pdf = (mpdr3 >> 8) & 0x3F;
		ret_val = ((clk_sel != 0) ? mxc_get_main_clock(CPU_CLK) :
			decode_pll(readl(&ccm->ppctl), MXC_HCLK)) / (pdf + 1);
		break;
	case ESDHC3_CLK:
		clk_sel = mpdr3 & 0x40;
		pdf = (mpdr3 >> 16) & 0x3F;
		ret_val = ((clk_sel != 0) ? mxc_get_main_clock(CPU_CLK) :
			decode_pll(readl(&ccm->ppctl), MXC_HCLK)) / (pdf + 1);
		break;
	case SPDIF_CLK:
		clk_sel = mpdr3 & 0x400000;
		pre_pdf = (mpdr3 >> 29) & 0x7;
		pdf = (mpdr3 >> 23) & 0x3F;
		ret_val = ((clk_sel != 0) ? mxc_get_main_clock(CPU_CLK) :
			decode_pll(readl(&ccm->ppctl), MXC_HCLK)) /
				((pre_pdf + 1) * (pdf + 1));
		break;
	default:
		printf("%s(): This clock: %d not supported yet\n",
				__func__, clk);
		break;
	}

	return ret_val;
}

unsigned int mxc_get_clock(enum mxc_clock clk)
{
	switch (clk) {
	case MXC_ARM_CLK:
		return get_mcu_main_clk();
	case MXC_AHB_CLK:
		break;
	case MXC_IPG_CLK:
		return get_ipg_clk();
	case MXC_IPG_PERCLK:
	case MXC_I2C_CLK:
		return get_ipg_per_clk();
	case MXC_UART_CLK:
		return imx_get_uartclk();
	case MXC_ESDHC1_CLK:
		return mxc_get_peri_clock(ESDHC1_CLK);
	case MXC_ESDHC2_CLK:
		return mxc_get_peri_clock(ESDHC2_CLK);
	case MXC_ESDHC3_CLK:
		return mxc_get_peri_clock(ESDHC3_CLK);
	case MXC_USB_CLK:
		return mxc_get_main_clock(USB_CLK);
	case MXC_FEC_CLK:
		return get_ipg_clk();
	case MXC_CSPI_CLK:
		return get_ipg_clk();
	}
	return -1;
}

#ifdef CONFIG_FEC_MXC
/*
 * The MX35 has no fuse for MAC, return a NULL MAC
 */
void imx_get_mac_from_fuse(int dev_id, unsigned char *mac)
{
	memset(mac, 0, 6);
}

u32 imx_get_fecclk(void)
{
	return mxc_get_clock(MXC_IPG_CLK);
}
#endif

int do_mx35_showclocks(cmd_tbl_t *cmdtp,
	int flag, int argc, char * const argv[])
{
	u32 cpufreq = get_mcu_main_clk();
	printf("mx35 cpu clock: %dMHz\n", cpufreq / 1000000);
	printf("ipg clock     : %dHz\n", get_ipg_clk());
	printf("ipg per clock : %dHz\n", get_ipg_per_clk());
	printf("uart clock    : %dHz\n", mxc_get_clock(MXC_UART_CLK));

	return 0;
}

U_BOOT_CMD(
	clocks,	CONFIG_SYS_MAXARGS, 1, do_mx35_showclocks,
	"display clocks",
	""
);

#if defined(CONFIG_DISPLAY_CPUINFO)
static char *get_reset_cause(void)
{
	/* read RCSR register from CCM module */
	struct ccm_regs *ccm =
		(struct ccm_regs *)IMX_CCM_BASE;

	u32 cause = readl(&ccm->rcsr) & 0x0F;

	switch (cause) {
	case 0x0000:
		return "POR";
	case 0x0002:
		return "JTAG";
	case 0x0004:
		return "RST";
	case 0x0008:
		return "WDOG";
	default:
		return "unknown reset";
	}
}

int print_cpuinfo(void)
{
	u32 srev = get_cpu_rev();

	printf("CPU:   Freescale i.MX35 rev %d.%d at %d MHz.\n",
		(srev & 0xF0) >> 4, (srev & 0x0F),
		get_mcu_main_clk() / 1000000);

	printf("Reset cause: %s\n", get_reset_cause());

	return 0;
}
#endif

/*
 * Initializes on-chip ethernet controllers.
 * to override, implement board_eth_init()
 */
int cpu_eth_init(bd_t *bis)
{
	int rc = -ENODEV;

#if defined(CONFIG_FEC_MXC)
	rc = fecmxc_initialize(bis);
#endif

	return rc;
}

#ifdef CONFIG_FSL_ESDHC
/*
 * Initializes on-chip MMC controllers.
 * to override, implement board_mmc_init()
 */
int cpu_mmc_init(bd_t *bis)
{
	return fsl_esdhc_mmc_init(bis);
}
#endif

int get_clocks(void)
{
#ifdef CONFIG_FSL_ESDHC
#if CONFIG_SYS_FSL_ESDHC_ADDR == MMC_SDHC2_BASE_ADDR
	gd->arch.sdhc_clk = mxc_get_clock(MXC_ESDHC2_CLK);
#elif CONFIG_SYS_FSL_ESDHC_ADDR == MMC_SDHC3_BASE_ADDR
	gd->arch.sdhc_clk = mxc_get_clock(MXC_ESDHC3_CLK);
#else
	gd->arch.sdhc_clk = mxc_get_clock(MXC_ESDHC1_CLK);
#endif
#endif
	return 0;
}

#define RCSR_MEM_CTL_WEIM	0
#define RCSR_MEM_CTL_NAND	1
#define RCSR_MEM_CTL_ATA	2
#define RCSR_MEM_CTL_EXPANSION	3
#define RCSR_MEM_TYPE_NOR	0
#define RCSR_MEM_TYPE_ONENAND	2
#define RCSR_MEM_TYPE_SD	0
#define RCSR_MEM_TYPE_I2C	2
#define RCSR_MEM_TYPE_SPI	3

u32 spl_boot_device(void)
{
	struct ccm_regs *ccm =
		(struct ccm_regs *)IMX_CCM_BASE;

	u32 rcsr = readl(&ccm->rcsr);
	u32 mem_type, mem_ctl;

	/* In external mode, no boot device is returned */
	if ((rcsr >> 10) & 0x03)
		return BOOT_DEVICE_NONE;

	mem_ctl = (rcsr >> 25) & 0x03;
	mem_type = (rcsr >> 23) & 0x03;

	switch (mem_ctl) {
	case RCSR_MEM_CTL_WEIM:
		switch (mem_type) {
		case RCSR_MEM_TYPE_NOR:
			return BOOT_DEVICE_NOR;
		case RCSR_MEM_TYPE_ONENAND:
			return BOOT_DEVICE_ONENAND;
		default:
			return BOOT_DEVICE_NONE;
		}
	case RCSR_MEM_CTL_NAND:
		return BOOT_DEVICE_NAND;
	case RCSR_MEM_CTL_EXPANSION:
		switch (mem_type) {
		case RCSR_MEM_TYPE_SD:
			return BOOT_DEVICE_MMC1;
		case RCSR_MEM_TYPE_I2C:
			return BOOT_DEVICE_I2C;
		case RCSR_MEM_TYPE_SPI:
			return BOOT_DEVICE_SPI;
		default:
			return BOOT_DEVICE_NONE;
		}
	}

	return BOOT_DEVICE_NONE;
}
