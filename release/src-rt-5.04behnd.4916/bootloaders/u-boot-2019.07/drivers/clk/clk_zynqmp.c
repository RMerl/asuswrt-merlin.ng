// SPDX-License-Identifier: GPL-2.0+
/*
 * ZynqMP clock driver
 *
 * Copyright (C) 2016 Xilinx, Inc.
 */

#include <common.h>
#include <linux/bitops.h>
#include <clk-uclass.h>
#include <clk.h>
#include <asm/arch/sys_proto.h>
#include <dm.h>

static const resource_size_t zynqmp_crf_apb_clkc_base = 0xfd1a0020;
static const resource_size_t zynqmp_crl_apb_clkc_base = 0xff5e0020;

/* Full power domain clocks */
#define CRF_APB_APLL_CTRL		(zynqmp_crf_apb_clkc_base + 0x00)
#define CRF_APB_DPLL_CTRL		(zynqmp_crf_apb_clkc_base + 0x0c)
#define CRF_APB_VPLL_CTRL		(zynqmp_crf_apb_clkc_base + 0x18)
#define CRF_APB_PLL_STATUS		(zynqmp_crf_apb_clkc_base + 0x24)
#define CRF_APB_APLL_TO_LPD_CTRL	(zynqmp_crf_apb_clkc_base + 0x28)
#define CRF_APB_DPLL_TO_LPD_CTRL	(zynqmp_crf_apb_clkc_base + 0x2c)
#define CRF_APB_VPLL_TO_LPD_CTRL	(zynqmp_crf_apb_clkc_base + 0x30)
/* Peripheral clocks */
#define CRF_APB_ACPU_CTRL		(zynqmp_crf_apb_clkc_base + 0x40)
#define CRF_APB_DBG_TRACE_CTRL		(zynqmp_crf_apb_clkc_base + 0x44)
#define CRF_APB_DBG_FPD_CTRL		(zynqmp_crf_apb_clkc_base + 0x48)
#define CRF_APB_DP_VIDEO_REF_CTRL	(zynqmp_crf_apb_clkc_base + 0x50)
#define CRF_APB_DP_AUDIO_REF_CTRL	(zynqmp_crf_apb_clkc_base + 0x54)
#define CRF_APB_DP_STC_REF_CTRL		(zynqmp_crf_apb_clkc_base + 0x5c)
#define CRF_APB_DDR_CTRL		(zynqmp_crf_apb_clkc_base + 0x60)
#define CRF_APB_GPU_REF_CTRL		(zynqmp_crf_apb_clkc_base + 0x64)
#define CRF_APB_SATA_REF_CTRL		(zynqmp_crf_apb_clkc_base + 0x80)
#define CRF_APB_PCIE_REF_CTRL		(zynqmp_crf_apb_clkc_base + 0x94)
#define CRF_APB_GDMA_REF_CTRL		(zynqmp_crf_apb_clkc_base + 0x98)
#define CRF_APB_DPDMA_REF_CTRL		(zynqmp_crf_apb_clkc_base + 0x9c)
#define CRF_APB_TOPSW_MAIN_CTRL		(zynqmp_crf_apb_clkc_base + 0xa0)
#define CRF_APB_TOPSW_LSBUS_CTRL	(zynqmp_crf_apb_clkc_base + 0xa4)
#define CRF_APB_GTGREF0_REF_CTRL	(zynqmp_crf_apb_clkc_base + 0xa8)
#define CRF_APB_DBG_TSTMP_CTRL		(zynqmp_crf_apb_clkc_base + 0xd8)

/* Low power domain clocks */
#define CRL_APB_IOPLL_CTRL		(zynqmp_crl_apb_clkc_base + 0x00)
#define CRL_APB_RPLL_CTRL		(zynqmp_crl_apb_clkc_base + 0x10)
#define CRL_APB_PLL_STATUS		(zynqmp_crl_apb_clkc_base + 0x20)
#define CRL_APB_IOPLL_TO_FPD_CTRL	(zynqmp_crl_apb_clkc_base + 0x24)
#define CRL_APB_RPLL_TO_FPD_CTRL	(zynqmp_crl_apb_clkc_base + 0x28)
/* Peripheral clocks */
#define CRL_APB_USB3_DUAL_REF_CTRL	(zynqmp_crl_apb_clkc_base + 0x2c)
#define CRL_APB_GEM0_REF_CTRL		(zynqmp_crl_apb_clkc_base + 0x30)
#define CRL_APB_GEM1_REF_CTRL		(zynqmp_crl_apb_clkc_base + 0x34)
#define CRL_APB_GEM2_REF_CTRL		(zynqmp_crl_apb_clkc_base + 0x38)
#define CRL_APB_GEM3_REF_CTRL		(zynqmp_crl_apb_clkc_base + 0x3c)
#define CRL_APB_USB0_BUS_REF_CTRL	(zynqmp_crl_apb_clkc_base + 0x40)
#define CRL_APB_USB1_BUS_REF_CTRL	(zynqmp_crl_apb_clkc_base + 0x44)
#define CRL_APB_QSPI_REF_CTRL		(zynqmp_crl_apb_clkc_base + 0x48)
#define CRL_APB_SDIO0_REF_CTRL		(zynqmp_crl_apb_clkc_base + 0x4c)
#define CRL_APB_SDIO1_REF_CTRL		(zynqmp_crl_apb_clkc_base + 0x50)
#define CRL_APB_UART0_REF_CTRL		(zynqmp_crl_apb_clkc_base + 0x54)
#define CRL_APB_UART1_REF_CTRL		(zynqmp_crl_apb_clkc_base + 0x58)
#define CRL_APB_SPI0_REF_CTRL		(zynqmp_crl_apb_clkc_base + 0x5c)
#define CRL_APB_SPI1_REF_CTRL		(zynqmp_crl_apb_clkc_base + 0x60)
#define CRL_APB_CAN0_REF_CTRL		(zynqmp_crl_apb_clkc_base + 0x64)
#define CRL_APB_CAN1_REF_CTRL		(zynqmp_crl_apb_clkc_base + 0x68)
#define CRL_APB_CPU_R5_CTRL		(zynqmp_crl_apb_clkc_base + 0x70)
#define CRL_APB_IOU_SWITCH_CTRL		(zynqmp_crl_apb_clkc_base + 0x7c)
#define CRL_APB_CSU_PLL_CTRL		(zynqmp_crl_apb_clkc_base + 0x80)
#define CRL_APB_PCAP_CTRL		(zynqmp_crl_apb_clkc_base + 0x84)
#define CRL_APB_LPD_SWITCH_CTRL		(zynqmp_crl_apb_clkc_base + 0x88)
#define CRL_APB_LPD_LSBUS_CTRL		(zynqmp_crl_apb_clkc_base + 0x8c)
#define CRL_APB_DBG_LPD_CTRL		(zynqmp_crl_apb_clkc_base + 0x90)
#define CRL_APB_NAND_REF_CTRL		(zynqmp_crl_apb_clkc_base + 0x94)
#define CRL_APB_ADMA_REF_CTRL		(zynqmp_crl_apb_clkc_base + 0x98)
#define CRL_APB_PL0_REF_CTRL		(zynqmp_crl_apb_clkc_base + 0xa0)
#define CRL_APB_PL1_REF_CTRL		(zynqmp_crl_apb_clkc_base + 0xa4)
#define CRL_APB_PL2_REF_CTRL		(zynqmp_crl_apb_clkc_base + 0xa8)
#define CRL_APB_PL3_REF_CTRL		(zynqmp_crl_apb_clkc_base + 0xac)
#define CRL_APB_PL0_THR_CNT		(zynqmp_crl_apb_clkc_base + 0xb4)
#define CRL_APB_PL1_THR_CNT		(zynqmp_crl_apb_clkc_base + 0xbc)
#define CRL_APB_PL2_THR_CNT		(zynqmp_crl_apb_clkc_base + 0xc4)
#define CRL_APB_PL3_THR_CNT		(zynqmp_crl_apb_clkc_base + 0xdc)
#define CRL_APB_GEM_TSU_REF_CTRL	(zynqmp_crl_apb_clkc_base + 0xe0)
#define CRL_APB_DLL_REF_CTRL		(zynqmp_crl_apb_clkc_base + 0xe4)
#define CRL_APB_AMS_REF_CTRL		(zynqmp_crl_apb_clkc_base + 0xe8)
#define CRL_APB_I2C0_REF_CTRL		(zynqmp_crl_apb_clkc_base + 0x100)
#define CRL_APB_I2C1_REF_CTRL		(zynqmp_crl_apb_clkc_base + 0x104)
#define CRL_APB_TIMESTAMP_REF_CTRL	(zynqmp_crl_apb_clkc_base + 0x108)

#define ZYNQ_CLK_MAXDIV		0x3f
#define CLK_CTRL_DIV1_SHIFT	16
#define CLK_CTRL_DIV1_MASK	(ZYNQ_CLK_MAXDIV << CLK_CTRL_DIV1_SHIFT)
#define CLK_CTRL_DIV0_SHIFT	8
#define CLK_CTRL_DIV0_MASK	(ZYNQ_CLK_MAXDIV << CLK_CTRL_DIV0_SHIFT)
#define CLK_CTRL_SRCSEL_SHIFT	0
#define CLK_CTRL_SRCSEL_MASK	(0x3 << CLK_CTRL_SRCSEL_SHIFT)
#define PLLCTRL_FBDIV_MASK	0x7f00
#define PLLCTRL_FBDIV_SHIFT	8
#define PLLCTRL_RESET_MASK	1
#define PLLCTRL_RESET_SHIFT	0
#define PLLCTRL_BYPASS_MASK	0x8
#define PLLCTRL_BYPASS_SHFT	3
#define PLLCTRL_POST_SRC_SHFT	24
#define PLLCTRL_POST_SRC_MASK	(0x7 << PLLCTRL_POST_SRC_SHFT)
#define PLLCTRL_PRE_SRC_SHFT	20
#define PLLCTRL_PRE_SRC_MASK	(0x7 << PLLCTRL_PRE_SRC_SHFT)


#define NUM_MIO_PINS	77

enum zynqmp_clk {
	iopll, rpll,
	apll, dpll, vpll,
	iopll_to_fpd, rpll_to_fpd, apll_to_lpd, dpll_to_lpd, vpll_to_lpd,
	acpu, acpu_half,
	dbg_fpd, dbg_lpd, dbg_trace, dbg_tstmp,
	dp_video_ref, dp_audio_ref,
	dp_stc_ref, gdma_ref, dpdma_ref,
	ddr_ref, sata_ref, pcie_ref,
	gpu_ref, gpu_pp0_ref, gpu_pp1_ref,
	topsw_main, topsw_lsbus,
	gtgref0_ref,
	lpd_switch, lpd_lsbus,
	usb0_bus_ref, usb1_bus_ref, usb3_dual_ref, usb0, usb1,
	cpu_r5, cpu_r5_core,
	csu_spb, csu_pll, pcap,
	iou_switch,
	gem_tsu_ref, gem_tsu,
	gem0_ref, gem1_ref, gem2_ref, gem3_ref,
	gem0_rx, gem1_rx, gem2_rx, gem3_rx,
	qspi_ref,
	sdio0_ref, sdio1_ref,
	uart0_ref, uart1_ref,
	spi0_ref, spi1_ref,
	nand_ref,
	i2c0_ref, i2c1_ref, can0_ref, can1_ref, can0, can1,
	dll_ref,
	adma_ref,
	timestamp_ref,
	ams_ref,
	pl0, pl1, pl2, pl3,
	wdt,
	clk_max,
};

static const char * const clk_names[clk_max] = {
	"iopll", "rpll", "apll", "dpll",
	"vpll", "iopll_to_fpd", "rpll_to_fpd",
	"apll_to_lpd", "dpll_to_lpd", "vpll_to_lpd",
	"acpu", "acpu_half", "dbf_fpd", "dbf_lpd",
	"dbg_trace", "dbg_tstmp", "dp_video_ref",
	"dp_audio_ref", "dp_stc_ref", "gdma_ref",
	"dpdma_ref", "ddr_ref", "sata_ref", "pcie_ref",
	"gpu_ref", "gpu_pp0_ref", "gpu_pp1_ref",
	"topsw_main", "topsw_lsbus", "gtgref0_ref",
	"lpd_switch", "lpd_lsbus", "usb0_bus_ref",
	"usb1_bus_ref", "usb3_dual_ref", "usb0",
	"usb1", "cpu_r5", "cpu_r5_core", "csu_spb",
	"csu_pll", "pcap", "iou_switch", "gem_tsu_ref",
	"gem_tsu", "gem0_ref", "gem1_ref", "gem2_ref",
	"gem3_ref", "gem0_tx", "gem1_tx", "gem2_tx",
	"gem3_tx", "qspi_ref", "sdio0_ref", "sdio1_ref",
	"uart0_ref", "uart1_ref", "spi0_ref",
	"spi1_ref", "nand_ref", "i2c0_ref", "i2c1_ref",
	"can0_ref", "can1_ref", "can0", "can1",
	"dll_ref", "adma_ref", "timestamp_ref",
	"ams_ref", "pl0", "pl1", "pl2", "pl3", "wdt"
};

struct zynqmp_clk_priv {
	unsigned long ps_clk_freq;
	unsigned long video_clk;
	unsigned long pss_alt_ref_clk;
	unsigned long gt_crx_ref_clk;
	unsigned long aux_ref_clk;
};

static u32 zynqmp_clk_get_register(enum zynqmp_clk id)
{
	switch (id) {
	case iopll:
		return CRL_APB_IOPLL_CTRL;
	case rpll:
		return CRL_APB_RPLL_CTRL;
	case apll:
		return CRF_APB_APLL_CTRL;
	case dpll:
		return CRF_APB_DPLL_CTRL;
	case vpll:
		return CRF_APB_VPLL_CTRL;
	case acpu:
		return CRF_APB_ACPU_CTRL;
	case ddr_ref:
		return CRF_APB_DDR_CTRL;
	case qspi_ref:
		return CRL_APB_QSPI_REF_CTRL;
	case gem0_ref:
		return CRL_APB_GEM0_REF_CTRL;
	case gem1_ref:
		return CRL_APB_GEM1_REF_CTRL;
	case gem2_ref:
		return CRL_APB_GEM2_REF_CTRL;
	case gem3_ref:
		return CRL_APB_GEM3_REF_CTRL;
	case uart0_ref:
		return CRL_APB_UART0_REF_CTRL;
	case uart1_ref:
		return CRL_APB_UART1_REF_CTRL;
	case sdio0_ref:
		return CRL_APB_SDIO0_REF_CTRL;
	case sdio1_ref:
		return CRL_APB_SDIO1_REF_CTRL;
	case spi0_ref:
		return CRL_APB_SPI0_REF_CTRL;
	case spi1_ref:
		return CRL_APB_SPI1_REF_CTRL;
	case nand_ref:
		return CRL_APB_NAND_REF_CTRL;
	case i2c0_ref:
		return CRL_APB_I2C0_REF_CTRL;
	case i2c1_ref:
		return CRL_APB_I2C1_REF_CTRL;
	case can0_ref:
		return CRL_APB_CAN0_REF_CTRL;
	case can1_ref:
		return CRL_APB_CAN1_REF_CTRL;
	case pl0:
		return CRL_APB_PL0_REF_CTRL;
	case pl1:
		return CRL_APB_PL1_REF_CTRL;
	case pl2:
		return CRL_APB_PL2_REF_CTRL;
	case pl3:
		return CRL_APB_PL3_REF_CTRL;
	case wdt:
		return CRF_APB_TOPSW_LSBUS_CTRL;
	case iopll_to_fpd:
		return CRL_APB_IOPLL_TO_FPD_CTRL;
	default:
		debug("Invalid clk id%d\n", id);
	}
	return 0;
}

static enum zynqmp_clk zynqmp_clk_get_cpu_pll(u32 clk_ctrl)
{
	u32 srcsel = (clk_ctrl & CLK_CTRL_SRCSEL_MASK) >>
		      CLK_CTRL_SRCSEL_SHIFT;

	switch (srcsel) {
	case 2:
		return dpll;
	case 3:
		return vpll;
	case 0 ... 1:
	default:
		return apll;
	}
}

static enum zynqmp_clk zynqmp_clk_get_ddr_pll(u32 clk_ctrl)
{
	u32 srcsel = (clk_ctrl & CLK_CTRL_SRCSEL_MASK) >>
		      CLK_CTRL_SRCSEL_SHIFT;

	switch (srcsel) {
	case 1:
		return vpll;
	case 0:
	default:
		return dpll;
	}
}

static enum zynqmp_clk zynqmp_clk_get_peripheral_pll(u32 clk_ctrl)
{
	u32 srcsel = (clk_ctrl & CLK_CTRL_SRCSEL_MASK) >>
		      CLK_CTRL_SRCSEL_SHIFT;

	switch (srcsel) {
	case 2:
		return rpll;
	case 3:
		return dpll;
	case 0 ... 1:
	default:
		return iopll;
	}
}

static enum zynqmp_clk zynqmp_clk_get_wdt_pll(u32 clk_ctrl)
{
	u32 srcsel = (clk_ctrl & CLK_CTRL_SRCSEL_MASK) >>
		      CLK_CTRL_SRCSEL_SHIFT;

	switch (srcsel) {
	case 2:
		return iopll_to_fpd;
	case 3:
		return dpll;
	case 0 ... 1:
	default:
		return apll;
	}
}

static ulong zynqmp_clk_get_pll_src(ulong clk_ctrl,
				    struct zynqmp_clk_priv *priv,
				    bool is_pre_src)
{
	u32 src_sel;

	if (is_pre_src)
		src_sel = (clk_ctrl & PLLCTRL_PRE_SRC_MASK) >>
			   PLLCTRL_PRE_SRC_SHFT;
	else
		src_sel = (clk_ctrl & PLLCTRL_POST_SRC_MASK) >>
			   PLLCTRL_POST_SRC_SHFT;

	switch (src_sel) {
	case 4:
		return priv->video_clk;
	case 5:
		return priv->pss_alt_ref_clk;
	case 6:
		return priv->aux_ref_clk;
	case 7:
		return priv->gt_crx_ref_clk;
	case 0 ... 3:
	default:
	return priv->ps_clk_freq;
	}
}

static ulong zynqmp_clk_get_pll_rate(struct zynqmp_clk_priv *priv,
				     enum zynqmp_clk id)
{
	u32 clk_ctrl, reset, mul;
	ulong freq;
	int ret;

	ret = zynqmp_mmio_read(zynqmp_clk_get_register(id), &clk_ctrl);
	if (ret) {
		printf("%s mio read fail\n", __func__);
		return -EIO;
	}

	if (clk_ctrl & PLLCTRL_BYPASS_MASK)
		freq = zynqmp_clk_get_pll_src(clk_ctrl, priv, 0);
	else
		freq = zynqmp_clk_get_pll_src(clk_ctrl, priv, 1);

	reset = (clk_ctrl & PLLCTRL_RESET_MASK) >> PLLCTRL_RESET_SHIFT;
	if (reset && !(clk_ctrl & PLLCTRL_BYPASS_MASK))
		return 0;

	mul = (clk_ctrl & PLLCTRL_FBDIV_MASK) >> PLLCTRL_FBDIV_SHIFT;

	freq *= mul;

	if (clk_ctrl & (1 << 16))
		freq /= 2;

	return freq;
}

static ulong zynqmp_clk_get_cpu_rate(struct zynqmp_clk_priv *priv,
				     enum zynqmp_clk id)
{
	u32 clk_ctrl, div;
	enum zynqmp_clk pll;
	int ret;
	unsigned long pllrate;

	ret = zynqmp_mmio_read(CRF_APB_ACPU_CTRL, &clk_ctrl);
	if (ret) {
		printf("%s mio read fail\n", __func__);
		return -EIO;
	}

	div = (clk_ctrl & CLK_CTRL_DIV0_MASK) >> CLK_CTRL_DIV0_SHIFT;

	pll = zynqmp_clk_get_cpu_pll(clk_ctrl);
	pllrate = zynqmp_clk_get_pll_rate(priv, pll);
	if (IS_ERR_VALUE(pllrate))
		return pllrate;

	return DIV_ROUND_CLOSEST(pllrate, div);
}

static ulong zynqmp_clk_get_ddr_rate(struct zynqmp_clk_priv *priv)
{
	u32 clk_ctrl, div;
	enum zynqmp_clk pll;
	int ret;
	ulong pllrate;

	ret = zynqmp_mmio_read(CRF_APB_DDR_CTRL, &clk_ctrl);
	if (ret) {
		printf("%s mio read fail\n", __func__);
		return -EIO;
	}

	div = (clk_ctrl & CLK_CTRL_DIV0_MASK) >> CLK_CTRL_DIV0_SHIFT;

	pll = zynqmp_clk_get_ddr_pll(clk_ctrl);
	pllrate = zynqmp_clk_get_pll_rate(priv, pll);
	if (IS_ERR_VALUE(pllrate))
		return pllrate;

	return DIV_ROUND_CLOSEST(pllrate, div);
}

static ulong zynqmp_clk_get_peripheral_rate(struct zynqmp_clk_priv *priv,
					  enum zynqmp_clk id, bool two_divs)
{
	enum zynqmp_clk pll;
	u32 clk_ctrl, div0;
	u32 div1 = 1;
	int ret;
	ulong pllrate;

	ret = zynqmp_mmio_read(zynqmp_clk_get_register(id), &clk_ctrl);
	if (ret) {
		printf("%s mio read fail\n", __func__);
		return -EIO;
	}

	div0 = (clk_ctrl & CLK_CTRL_DIV0_MASK) >> CLK_CTRL_DIV0_SHIFT;
	if (!div0)
		div0 = 1;

	if (two_divs) {
		div1 = (clk_ctrl & CLK_CTRL_DIV1_MASK) >> CLK_CTRL_DIV1_SHIFT;
		if (!div1)
			div1 = 1;
	}

	pll = zynqmp_clk_get_peripheral_pll(clk_ctrl);
	pllrate = zynqmp_clk_get_pll_rate(priv, pll);
	if (IS_ERR_VALUE(pllrate))
		return pllrate;

	return
		DIV_ROUND_CLOSEST(
			DIV_ROUND_CLOSEST(pllrate, div0), div1);
}

static ulong zynqmp_clk_get_wdt_rate(struct zynqmp_clk_priv *priv,
				     enum zynqmp_clk id, bool two_divs)
{
	enum zynqmp_clk pll;
	u32 clk_ctrl, div0;
	u32 div1 = 1;
	int ret;
	ulong pllrate;

	ret = zynqmp_mmio_read(zynqmp_clk_get_register(id), &clk_ctrl);
	if (ret) {
		printf("%d %s mio read fail\n", __LINE__, __func__);
		return -EIO;
	}

	div0 = (clk_ctrl & CLK_CTRL_DIV0_MASK) >> CLK_CTRL_DIV0_SHIFT;
	if (!div0)
		div0 = 1;

	pll = zynqmp_clk_get_wdt_pll(clk_ctrl);
	if (two_divs) {
		ret = zynqmp_mmio_read(zynqmp_clk_get_register(pll), &clk_ctrl);
		if (ret) {
			printf("%d %s mio read fail\n", __LINE__, __func__);
			return -EIO;
		}
		div1 = (clk_ctrl & CLK_CTRL_DIV0_MASK) >> CLK_CTRL_DIV0_SHIFT;
		if (!div1)
			div1 = 1;
	}

	if (pll == iopll_to_fpd)
		pll = iopll;

	pllrate = zynqmp_clk_get_pll_rate(priv, pll);
	if (IS_ERR_VALUE(pllrate))
		return pllrate;

	return
		DIV_ROUND_CLOSEST(
			DIV_ROUND_CLOSEST(pllrate, div0), div1);
}

static unsigned long zynqmp_clk_calc_peripheral_two_divs(ulong rate,
						       ulong pll_rate,
						       u32 *div0, u32 *div1)
{
	long new_err, best_err = (long)(~0UL >> 1);
	ulong new_rate, best_rate = 0;
	u32 d0, d1;

	for (d0 = 1; d0 <= ZYNQ_CLK_MAXDIV; d0++) {
		for (d1 = 1; d1 <= ZYNQ_CLK_MAXDIV >> 1; d1++) {
			new_rate = DIV_ROUND_CLOSEST(
					DIV_ROUND_CLOSEST(pll_rate, d0), d1);
			new_err = abs(new_rate - rate);

			if (new_err < best_err) {
				*div0 = d0;
				*div1 = d1;
				best_err = new_err;
				best_rate = new_rate;
			}
		}
	}

	return best_rate;
}

static ulong zynqmp_clk_set_peripheral_rate(struct zynqmp_clk_priv *priv,
					  enum zynqmp_clk id, ulong rate,
					  bool two_divs)
{
	enum zynqmp_clk pll;
	u32 clk_ctrl, div0 = 0, div1 = 0;
	ulong pll_rate, new_rate;
	u32 reg;
	int ret;
	u32 mask;

	reg = zynqmp_clk_get_register(id);
	ret = zynqmp_mmio_read(reg, &clk_ctrl);
	if (ret) {
		printf("%s mio read fail\n", __func__);
		return -EIO;
	}

	pll = zynqmp_clk_get_peripheral_pll(clk_ctrl);
	pll_rate = zynqmp_clk_get_pll_rate(priv, pll);
	if (IS_ERR_VALUE(pll_rate))
		return pll_rate;

	clk_ctrl &= ~CLK_CTRL_DIV0_MASK;
	if (two_divs) {
		clk_ctrl &= ~CLK_CTRL_DIV1_MASK;
		new_rate = zynqmp_clk_calc_peripheral_two_divs(rate, pll_rate,
				&div0, &div1);
		clk_ctrl |= div1 << CLK_CTRL_DIV1_SHIFT;
	} else {
		div0 = DIV_ROUND_CLOSEST(pll_rate, rate);
		if (div0 > ZYNQ_CLK_MAXDIV)
			div0 = ZYNQ_CLK_MAXDIV;
		new_rate = DIV_ROUND_CLOSEST(rate, div0);
	}
	clk_ctrl |= div0 << CLK_CTRL_DIV0_SHIFT;

	mask = (ZYNQ_CLK_MAXDIV << CLK_CTRL_DIV0_SHIFT) |
	       (ZYNQ_CLK_MAXDIV << CLK_CTRL_DIV1_SHIFT);

	ret = zynqmp_mmio_write(reg, mask, clk_ctrl);
	if (ret) {
		printf("%s mio write fail\n", __func__);
		return -EIO;
	}

	return new_rate;
}

static ulong zynqmp_clk_get_rate(struct clk *clk)
{
	struct zynqmp_clk_priv *priv = dev_get_priv(clk->dev);
	enum zynqmp_clk id = clk->id;
	bool two_divs = false;

	switch (id) {
	case iopll ... vpll:
		return zynqmp_clk_get_pll_rate(priv, id);
	case acpu:
		return zynqmp_clk_get_cpu_rate(priv, id);
	case ddr_ref:
		return zynqmp_clk_get_ddr_rate(priv);
	case gem0_ref ... gem3_ref:
	case qspi_ref ... can1_ref:
	case pl0 ... pl3:
		two_divs = true;
		return zynqmp_clk_get_peripheral_rate(priv, id, two_divs);
	case wdt:
		two_divs = true;
		return zynqmp_clk_get_wdt_rate(priv, id, two_divs);
	default:
		return -ENXIO;
	}
}

static ulong zynqmp_clk_set_rate(struct clk *clk, ulong rate)
{
	struct zynqmp_clk_priv *priv = dev_get_priv(clk->dev);
	enum zynqmp_clk id = clk->id;
	bool two_divs = true;

	switch (id) {
	case gem0_ref ... gem3_ref:
	case qspi_ref ... can1_ref:
		return zynqmp_clk_set_peripheral_rate(priv, id,
						      rate, two_divs);
	default:
		return -ENXIO;
	}
}

int soc_clk_dump(void)
{
	struct udevice *dev;
	int i, ret;

	ret = uclass_get_device_by_driver(UCLASS_CLK,
		DM_GET_DRIVER(zynqmp_clk), &dev);
	if (ret)
		return ret;

	printf("clk\t\tfrequency\n");
	for (i = 0; i < clk_max; i++) {
		const char *name = clk_names[i];
		if (name) {
			struct clk clk;
			unsigned long rate;

			clk.id = i;
			ret = clk_request(dev, &clk);
			if (ret < 0)
				return ret;

			rate = clk_get_rate(&clk);

			clk_free(&clk);

			if ((rate == (unsigned long)-ENOSYS) ||
			    (rate == (unsigned long)-ENXIO) ||
			    (rate == (unsigned long)-EIO))
				printf("%10s%20s\n", name, "unknown");
			else
				printf("%10s%20lu\n", name, rate);
		}
	}

	return 0;
}

static int zynqmp_get_freq_by_name(char *name, struct udevice *dev, ulong *freq)
{
	struct clk clk;
	int ret;

	ret = clk_get_by_name(dev, name, &clk);
	if (ret < 0) {
		dev_err(dev, "failed to get %s\n", name);
		return ret;
	}

	*freq = clk_get_rate(&clk);
	if (IS_ERR_VALUE(*freq)) {
		dev_err(dev, "failed to get rate %s\n", name);
		return -EINVAL;
	}

	return 0;
}
static int zynqmp_clk_probe(struct udevice *dev)
{
	int ret;
	struct zynqmp_clk_priv *priv = dev_get_priv(dev);

	debug("%s\n", __func__);
	ret = zynqmp_get_freq_by_name("pss_ref_clk", dev, &priv->ps_clk_freq);
	if (ret < 0)
		return -EINVAL;

	ret = zynqmp_get_freq_by_name("video_clk", dev, &priv->video_clk);
	if (ret < 0)
		return -EINVAL;

	ret = zynqmp_get_freq_by_name("pss_alt_ref_clk", dev,
				      &priv->pss_alt_ref_clk);
	if (ret < 0)
		return -EINVAL;

	ret = zynqmp_get_freq_by_name("aux_ref_clk", dev, &priv->aux_ref_clk);
	if (ret < 0)
		return -EINVAL;

	ret = zynqmp_get_freq_by_name("gt_crx_ref_clk", dev,
				      &priv->gt_crx_ref_clk);
	if (ret < 0)
		return -EINVAL;

	return 0;
}

static struct clk_ops zynqmp_clk_ops = {
	.set_rate = zynqmp_clk_set_rate,
	.get_rate = zynqmp_clk_get_rate,
};

static const struct udevice_id zynqmp_clk_ids[] = {
	{ .compatible = "xlnx,zynqmp-clk" },
	{ .compatible = "xlnx,zynqmp-clkc" },
	{ }
};

U_BOOT_DRIVER(zynqmp_clk) = {
	.name = "zynqmp-clk",
	.id = UCLASS_CLK,
	.of_match = zynqmp_clk_ids,
	.probe = zynqmp_clk_probe,
	.ops = &zynqmp_clk_ops,
	.priv_auto_alloc_size = sizeof(struct zynqmp_clk_priv),
};
