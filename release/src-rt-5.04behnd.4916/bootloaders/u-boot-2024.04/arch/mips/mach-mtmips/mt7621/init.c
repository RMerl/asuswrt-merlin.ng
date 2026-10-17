// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2022 MediaTek Inc. All rights reserved.
 *
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 */

#include <clk.h>
#include <dm.h>
#include <dm/uclass.h>
#include <dt-bindings/clock/mt7621-clk.h>
#include <asm/global_data.h>
#include <linux/io.h>
#include <linux/bitfield.h>
#include "mt7621.h"

DECLARE_GLOBAL_DATA_PTR;

static const char *const boot_mode[(CHIP_MODE_M >> CHIP_MODE_S) + 1] = {
	[1] = "NAND 2K+64",
	[2] = "SPI-NOR 3-Byte Addr",
	[3] = "SPI-NOR 4-Byte Addr",
	[10] = "NAND 2K+128",
	[11] = "NAND 4K+128",
	[12] = "NAND 4K+256",
};

int print_cpuinfo(void)
{
	void __iomem *sysc = ioremap_nocache(SYSCTL_BASE, SYSCTL_SIZE);
	u32 val, ver, eco, pkg, core, dram, chipmode;
	u32 cpu_clk, ddr_clk, bus_clk, xtal_clk;
	struct udevice *clkdev;
	const char *bootdev;
	struct clk clk;
	int ret;

	val = readl(sysc + SYSCTL_CHIP_REV_ID_REG);
	ver = FIELD_GET(VER_ID_M, val);
	eco = FIELD_GET(ECO_ID_M, val);
	pkg = FIELD_GET(PKG_ID, val);
	core = FIELD_GET(CPU_ID, val);

	val = readl(sysc + SYSCTL_SYSCFG0_REG);
	dram = FIELD_GET(DRAM_TYPE, val);
	chipmode = FIELD_GET(CHIP_MODE_M, val);

	bootdev = boot_mode[chipmode];
	if (!bootdev)
		bootdev = "Unsupported boot mode";

	printf("CPU:   MediaTek MT7621%c ver %u, eco %u\n",
	       core ? (pkg ? 'A' : 'N') : 'S', ver, eco);

	printf("Boot:  DDR%u, %s\n", dram ? 2 : 3, bootdev);

	ret = uclass_get_device_by_driver(UCLASS_CLK, DM_DRIVER_GET(mt7621_clk),
					  &clkdev);
	if (ret)
		return ret;

	clk.dev = clkdev;

	clk.id = MT7621_CLK_CPU;
	cpu_clk = clk_get_rate(&clk);

	clk.id = MT7621_CLK_BUS;
	bus_clk = clk_get_rate(&clk);

	clk.id = MT7621_CLK_DDR;
	ddr_clk = clk_get_rate(&clk);

	clk.id = MT7621_CLK_XTAL;
	xtal_clk = clk_get_rate(&clk);

	/* Set final timer frequency */
	if (cpu_clk)
		gd->arch.timer_freq = cpu_clk / 2;

	printf("Clock: CPU: %uMHz, DDR: %uMT/s, Bus: %uMHz, XTAL: %uMHz\n",
	       cpu_clk / 1000000, ddr_clk / 500000, bus_clk / 1000000,
	       xtal_clk / 1000000);

	return 0;
}

unsigned long get_xtal_mhz(void)
{
	void __iomem *sysc = ioremap_nocache(SYSCTL_BASE, SYSCTL_SIZE);
	u32 bs, xtal_sel;

	bs = readl(sysc + SYSCTL_SYSCFG0_REG);
	xtal_sel = FIELD_GET(XTAL_MODE_SEL_M, bs);

	if (xtal_sel <= 2)
		return 20;
	else if (xtal_sel <= 5)
		return 40;
	else
		return 25;
}

static void xhci_config_40mhz(void __iomem *usbh)
{
	writel(FIELD_PREP(SSUSB_MAC3_SYS_CK_GATE_MASK_TIME_M, 0x20) |
	       FIELD_PREP(SSUSB_MAC2_SYS_CK_GATE_MASK_TIME_M, 0x20) |
	       FIELD_PREP(SSUSB_MAC3_SYS_CK_GATE_MODE_M, 2) |
	       FIELD_PREP(SSUSB_MAC2_SYS_CK_GATE_MODE_M, 2) | 0x10,
	       usbh + SSUSB_MAC_CK_CTRL_REG);

	writel(FIELD_PREP(SSUSB_PLL_PREDIV_PE1D_M, 2) |
	       FIELD_PREP(SSUSB_PLL_PREDIV_U3_M, 1) |
	       FIELD_PREP(SSUSB_PLL_FBKDI_M, 4),
	       usbh + DA_SSUSB_U3PHYA_10_REG);

	writel(FIELD_PREP(SSUSB_PLL_FBKDIV_PE2H_M, 0x18) |
	       FIELD_PREP(SSUSB_PLL_FBKDIV_PE1D_M, 0x18) |
	       FIELD_PREP(SSUSB_PLL_FBKDIV_PE1H_M, 0x18) |
	       FIELD_PREP(SSUSB_PLL_FBKDIV_U3_M, 0x1e),
	       usbh + DA_SSUSB_PLL_FBKDIV_REG);

	writel(FIELD_PREP(SSUSB_PLL_PCW_NCPO_U3_M, 0x1e400000),
	       usbh + DA_SSUSB_PLL_PCW_NCPO_REG);

	writel(FIELD_PREP(SSUSB_PLL_SSC_DELTA1_PE1H_M, 0x25) |
	       FIELD_PREP(SSUSB_PLL_SSC_DELTA1_U3_M, 0x73),
	       usbh + DA_SSUSB_PLL_SSC_DELTA1_REG);

	writel(FIELD_PREP(SSUSB_PLL_SSC_DELTA_U3_M, 0x71) |
	       FIELD_PREP(SSUSB_PLL_SSC_DELTA1_PE2D_M, 0x4a),
	       usbh + DA_SSUSB_U3PHYA_21_REG);

	writel(FIELD_PREP(SSUSB_PLL_SSC_PRD_M, 0x140),
	       usbh + SSUSB_U3PHYA_9_REG);

	writel(FIELD_PREP(SSUSB_SYSPLL_PCW_NCPO_M, 0x11c00000),
	       usbh + SSUSB_U3PHYA_3_REG);

	writel(FIELD_PREP(SSUSB_PCIE_CLKDRV_AMP_M, 4) |
	       FIELD_PREP(SSUSB_SYSPLL_FBSEL_M, 1) |
	       FIELD_PREP(SSUSB_SYSPLL_PREDIV_M, 1),
	       usbh + SSUSB_U3PHYA_1_REG);

	writel(FIELD_PREP(SSUSB_SYSPLL_FBDIV_M, 0x12) |
	       SSUSB_SYSPLL_VCO_DIV_SEL | SSUSB_SYSPLL_FPEN |
	       SSUSB_SYSPLL_MONCK_EN | SSUSB_SYSPLL_VOD_EN,
	       usbh + SSUSB_U3PHYA_2_REG);

	writel(SSUSB_EQ_CURSEL | FIELD_PREP(SSUSB_RX_DAC_MUX_M, 8) |
	       FIELD_PREP(SSUSB_PCIE_SIGDET_VTH_M, 1) |
	       FIELD_PREP(SSUSB_PCIE_SIGDET_LPF_M, 1),
	       usbh + SSUSB_U3PHYA_11_REG);

	writel(FIELD_PREP(SSUSB_RING_OSC_CNTEND_M, 0x1ff) |
	       FIELD_PREP(SSUSB_XTAL_OSC_CNTEND_M, 0x7f) |
	       SSUSB_RING_BYPASS_DET,
	       usbh + SSUSB_B2_ROSC_0_REG);

	writel(FIELD_PREP(SSUSB_RING_OSC_FRC_RECAL_M, 3) |
	       SSUSB_RING_OSC_FRC_SEL,
	       usbh + SSUSB_B2_ROSC_1_REG);
}

static void xhci_config_25mhz(void __iomem *usbh)
{
	writel(FIELD_PREP(SSUSB_MAC3_SYS_CK_GATE_MASK_TIME_M, 0x20) |
	       FIELD_PREP(SSUSB_MAC2_SYS_CK_GATE_MASK_TIME_M, 0x20) |
	       FIELD_PREP(SSUSB_MAC3_SYS_CK_GATE_MODE_M, 2) |
	       FIELD_PREP(SSUSB_MAC2_SYS_CK_GATE_MODE_M, 2) | 0x10,
	       usbh + SSUSB_MAC_CK_CTRL_REG);

	writel(FIELD_PREP(SSUSB_PLL_PREDIV_PE1D_M, 2) |
	       FIELD_PREP(SSUSB_PLL_FBKDI_M, 4),
	       usbh + DA_SSUSB_U3PHYA_10_REG);

	writel(FIELD_PREP(SSUSB_PLL_FBKDIV_PE2H_M, 0x18) |
	       FIELD_PREP(SSUSB_PLL_FBKDIV_PE1D_M, 0x18) |
	       FIELD_PREP(SSUSB_PLL_FBKDIV_PE1H_M, 0x18) |
	       FIELD_PREP(SSUSB_PLL_FBKDIV_U3_M, 0x19),
	       usbh + DA_SSUSB_PLL_FBKDIV_REG);

	writel(FIELD_PREP(SSUSB_PLL_PCW_NCPO_U3_M, 0x18000000),
	       usbh + DA_SSUSB_PLL_PCW_NCPO_REG);

	writel(FIELD_PREP(SSUSB_PLL_SSC_DELTA1_PE1H_M, 0x25) |
	       FIELD_PREP(SSUSB_PLL_SSC_DELTA1_U3_M, 0x4a),
	       usbh + DA_SSUSB_PLL_SSC_DELTA1_REG);

	writel(FIELD_PREP(SSUSB_PLL_SSC_DELTA_U3_M, 0x48) |
	       FIELD_PREP(SSUSB_PLL_SSC_DELTA1_PE2D_M, 0x4a),
	       usbh + DA_SSUSB_U3PHYA_21_REG);

	writel(FIELD_PREP(SSUSB_PLL_SSC_PRD_M, 0x190),
	       usbh + SSUSB_U3PHYA_9_REG);

	writel(FIELD_PREP(SSUSB_SYSPLL_PCW_NCPO_M, 0xe000000),
	       usbh + SSUSB_U3PHYA_3_REG);

	writel(FIELD_PREP(SSUSB_PCIE_CLKDRV_AMP_M, 4) |
	       FIELD_PREP(SSUSB_SYSPLL_FBSEL_M, 1),
	       usbh + SSUSB_U3PHYA_1_REG);

	writel(FIELD_PREP(SSUSB_SYSPLL_FBDIV_M, 0xf) |
	       SSUSB_SYSPLL_VCO_DIV_SEL | SSUSB_SYSPLL_FPEN |
	       SSUSB_SYSPLL_MONCK_EN | SSUSB_SYSPLL_VOD_EN,
	       usbh + SSUSB_U3PHYA_2_REG);

	writel(SSUSB_EQ_CURSEL | FIELD_PREP(SSUSB_RX_DAC_MUX_M, 8) |
	       FIELD_PREP(SSUSB_PCIE_SIGDET_VTH_M, 1) |
	       FIELD_PREP(SSUSB_PCIE_SIGDET_LPF_M, 1),
	       usbh + SSUSB_U3PHYA_11_REG);

	writel(FIELD_PREP(SSUSB_RING_OSC_CNTEND_M, 0x1ff) |
	       FIELD_PREP(SSUSB_XTAL_OSC_CNTEND_M, 0x7f) |
	       SSUSB_RING_BYPASS_DET,
	       usbh + SSUSB_B2_ROSC_0_REG);

	writel(FIELD_PREP(SSUSB_RING_OSC_FRC_RECAL_M, 3) |
	       SSUSB_RING_OSC_FRC_SEL,
	       usbh + SSUSB_B2_ROSC_1_REG);
}

void lowlevel_init(void)
{
	void __iomem *usbh = ioremap_nocache(SSUSB_BASE, SSUSB_SIZE);
	u32 xtal = get_xtal_mhz();

	/* Setup USB xHCI */
	if (xtal == 40)
		xhci_config_40mhz(usbh);
	else if (xtal == 25)
		xhci_config_25mhz(usbh);
}

ulong notrace get_tbclk(void)
{
	return gd->arch.timer_freq;
}

void _machine_restart(void)
{
	void __iomem *sysc = ioremap_nocache(SYSCTL_BASE, SYSCTL_SIZE);

	while (1)
		writel(SYS_RST, sysc + SYSCTL_RSTCTL_REG);
}
