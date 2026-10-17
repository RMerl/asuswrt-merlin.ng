// SPDX-License-Identifier: GPL-2.0
/*
 * Octeon family DWC3 specific glue layer
 *
 * Copyright (C) 2020 Stefan Roese <sr@denx.de>
 *
 * The low-level init code is based on the Linux driver octeon-usb.c by
 * David Daney <david.daney@cavium.com>, which is:
 * Copyright (C) 2010-2017 Cavium Networks
 */

#include <dm.h>
#include <errno.h>
#include <usb.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <dm/device_compat.h>
#include <dm/lists.h>
#include <dm/of_access.h>
#include <linux/bitfield.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/usb/dwc3.h>
#include <linux/usb/otg.h>
#include <mach/octeon-model.h>

DECLARE_GLOBAL_DATA_PTR;

#define CVMX_GPIO_BIT_CFGX(i)	(0x0001070000000900ull + ((i) * 8))
#define CVMX_GPIO_XBIT_CFGX(i)	(0x0001070000000900ull + \
				 ((i) & 31) * 8 - 8 * 16)

#define GPIO_BIT_CFG_TX_OE		BIT_ULL(0)
#define GPIO_BIT_CFG_OUTPUT_SEL		GENMASK_ULL(20, 16)

#define UCTL_CTL_UCTL_RST		BIT_ULL(0)
#define UCTL_CTL_UAHC_RST		BIT_ULL(1)
#define UCTL_CTL_UPHY_RST		BIT_ULL(2)
#define UCTL_CTL_DRD_MODE		BIT_ULL(3)
#define UCTL_CTL_SCLK_EN		BIT_ULL(4)
#define UCTL_CTL_HS_POWER_EN		BIT_ULL(12)
#define UCTL_CTL_SS_POWER_EN		BIT_ULL(14)
#define UCTL_CTL_H_CLKDIV_SEL		GENMASK_ULL(26, 24)
#define UCTL_CTL_H_CLKDIV_RST		BIT_ULL(28)
#define UCTL_CTL_H_CLK_EN		BIT_ULL(30)
#define UCTL_CTL_REF_CLK_FSEL		GENMASK_ULL(37, 32)
#define UCTL_CTL_REF_CLK_DIV2		BIT_ULL(38)
#define UCTL_CTL_REF_SSP_EN		BIT_ULL(39)
#define UCTL_CTL_MPLL_MULTIPLIER	GENMASK_ULL(46, 40)
#define UCTL_CTL_SSC_EN			BIT_ULL(59)
#define UCTL_CTL_REF_CLK_SEL		GENMASK_ULL(61, 60)

#define UCTL_HOST_CFG			0xe0
#define UCTL_HOST_CFG_PPC_ACTIVE_HIGH_EN BIT_ULL(24)
#define UCTL_HOST_CFG_PPC_EN		BIT_ULL(25)

#define UCTL_SHIM_CFG			0xe8
#define UCTL_SHIM_CFG_CSR_ENDIAN_MODE	GENMASK_ULL(1, 0)
#define UCTL_SHIM_CFG_DMA_ENDIAN_MODE	GENMASK_ULL(9, 8)

#define OCTEON_H_CLKDIV_SEL		8
#define OCTEON_MIN_H_CLK_RATE		150000000
#define OCTEON_MAX_H_CLK_RATE		300000000

#define CLOCK_50MHZ			50000000
#define CLOCK_100MHZ			100000000
#define CLOCK_125MHZ			125000000

static u8 clk_div[OCTEON_H_CLKDIV_SEL] = {1, 2, 4, 6, 8, 16, 24, 32};

static int dwc3_octeon_config_power(struct udevice *dev, void __iomem *base)
{
	u64 uctl_host_cfg;
	u64 gpio_bit;
	u32 gpio_pwr[3];
	int gpio, len, power_active_low;
	const struct device_node *node = dev_np(dev);
	int index = ((u64)base >> 24) & 1;
	void __iomem *gpio_bit_cfg;

	if (of_find_property(node, "power", &len)) {
		if (len == 12) {
			dev_read_u32_array(dev, "power", gpio_pwr, 3);
			power_active_low = gpio_pwr[2] & 0x01;
			gpio = gpio_pwr[1];
		} else if (len == 8) {
			dev_read_u32_array(dev, "power", gpio_pwr, 2);
			power_active_low = 0;
			gpio = gpio_pwr[1];
		} else {
			printf("dwc3 controller clock init failure\n");
			return -EINVAL;
		}

		gpio_bit_cfg = ioremap(CVMX_GPIO_BIT_CFGX(gpio), 0);

		if ((OCTEON_IS_MODEL(OCTEON_CN73XX) ||
		     OCTEON_IS_MODEL(OCTEON_CNF75XX)) && gpio <= 31) {
			gpio_bit = ioread64(gpio_bit_cfg);
			gpio_bit |= GPIO_BIT_CFG_TX_OE;
			gpio_bit &= ~GPIO_BIT_CFG_OUTPUT_SEL;
			gpio_bit |= FIELD_PREP(GPIO_BIT_CFG_OUTPUT_SEL,
					       index == 0 ? 0x14 : 0x15);
			iowrite64(gpio_bit, gpio_bit_cfg);
		} else if (gpio <= 15) {
			gpio_bit = ioread64(gpio_bit_cfg);
			gpio_bit |= GPIO_BIT_CFG_TX_OE;
			gpio_bit &= ~GPIO_BIT_CFG_OUTPUT_SEL;
			gpio_bit |= FIELD_PREP(GPIO_BIT_CFG_OUTPUT_SEL,
					       index == 0 ? 0x14 : 0x19);
			iowrite64(gpio_bit, gpio_bit_cfg);
		} else {
			gpio_bit_cfg = ioremap(CVMX_GPIO_XBIT_CFGX(gpio), 0);

			gpio_bit = ioread64(gpio_bit_cfg);
			gpio_bit |= GPIO_BIT_CFG_TX_OE;
			gpio_bit &= ~GPIO_BIT_CFG_OUTPUT_SEL;
			gpio_bit |= FIELD_PREP(GPIO_BIT_CFG_OUTPUT_SEL,
					       index == 0 ? 0x14 : 0x19);
			iowrite64(gpio_bit, gpio_bit_cfg);
		}

		/* Enable XHCI power control and set if active high or low. */
		uctl_host_cfg = ioread64(base + UCTL_HOST_CFG);
		uctl_host_cfg |= UCTL_HOST_CFG_PPC_EN;
		if (power_active_low)
			uctl_host_cfg &= ~UCTL_HOST_CFG_PPC_ACTIVE_HIGH_EN;
		else
			uctl_host_cfg |= UCTL_HOST_CFG_PPC_ACTIVE_HIGH_EN;
		iowrite64(uctl_host_cfg, base + UCTL_HOST_CFG);

		/* Wait for power to stabilize */
		mdelay(10);
	} else {
		/* Disable XHCI power control and set if active high. */
		uctl_host_cfg = ioread64(base + UCTL_HOST_CFG);
		uctl_host_cfg &= ~UCTL_HOST_CFG_PPC_EN;
		uctl_host_cfg &= ~UCTL_HOST_CFG_PPC_ACTIVE_HIGH_EN;
		iowrite64(uctl_host_cfg, base + UCTL_HOST_CFG);
		dev_warn(dev, "dwc3 controller clock init failure.\n");
	}

	return 0;
}

static int dwc3_octeon_clocks_start(struct udevice *dev, void __iomem *base)
{
	u64 uctl_ctl;
	int ref_clk_sel = 2;
	u64 div;
	u32 clock_rate;
	int mpll_mul;
	int i;
	u64 h_clk_rate;
	void __iomem *uctl_ctl_reg = base;
	const char *ss_clock_type;
	const char *hs_clock_type;

	i = dev_read_u32(dev, "refclk-frequency", &clock_rate);
	if (i) {
		printf("No UCTL \"refclk-frequency\"\n");
		return -EINVAL;
	}

	ss_clock_type = dev_read_string(dev, "refclk-type-ss");
	if (!ss_clock_type) {
		printf("No UCTL \"refclk-type-ss\"\n");
		return -EINVAL;
	}

	hs_clock_type = dev_read_string(dev, "refclk-type-hs");
	if (!hs_clock_type) {
		printf("No UCTL \"refclk-type-hs\"\n");
		return -EINVAL;
	}

	if (strcmp("dlmc_ref_clk0", ss_clock_type) == 0) {
		if (strcmp(hs_clock_type, "dlmc_ref_clk0") == 0) {
			ref_clk_sel = 0;
		} else if (strcmp(hs_clock_type, "pll_ref_clk") == 0) {
			ref_clk_sel = 2;
		} else {
			printf("Invalid HS clock type %s, using pll_ref_clk\n",
			       hs_clock_type);
		}
	} else if (strcmp(ss_clock_type, "dlmc_ref_clk1") == 0) {
		if (strcmp(hs_clock_type, "dlmc_ref_clk1") == 0) {
			ref_clk_sel = 1;
		} else if (strcmp(hs_clock_type, "pll_ref_clk") == 0) {
			ref_clk_sel = 3;
		} else {
			printf("Invalid HS clock type %s, using pll_ref_clk\n",
			       hs_clock_type);
			ref_clk_sel = 3;
		}
	} else {
		printf("Invalid SS clock type %s, using dlmc_ref_clk0\n",
		       ss_clock_type);
	}

	if ((ref_clk_sel == 0 || ref_clk_sel == 1) &&
	    clock_rate != CLOCK_100MHZ)
		printf("Invalid UCTL clock rate of %u\n", clock_rate);

	/*
	 * Step 1: Wait for all voltages to be stable...that surely
	 *         happened before this driver is started. SKIP
	 */

	/* Step 2: Select GPIO for overcurrent indication, if desired. SKIP */

	/* Step 3: Assert all resets. */
	uctl_ctl = ioread64(uctl_ctl_reg);
	uctl_ctl |= UCTL_CTL_UCTL_RST | UCTL_CTL_UAHC_RST | UCTL_CTL_UPHY_RST;
	iowrite64(uctl_ctl, uctl_ctl_reg);

	/* Step 4a: Reset the clock dividers. */
	uctl_ctl = ioread64(uctl_ctl_reg);
	uctl_ctl |= UCTL_CTL_H_CLKDIV_RST;
	iowrite64(uctl_ctl, uctl_ctl_reg);

	/* Step 4b: Select controller clock frequency. */
	for (div = ARRAY_SIZE(clk_div) - 1; div >= 0; div--) {
		h_clk_rate = gd->bus_clk / clk_div[div];
		if (h_clk_rate <= OCTEON_MAX_H_CLK_RATE &&
		    h_clk_rate >= OCTEON_MIN_H_CLK_RATE)
			break;
	}
	uctl_ctl = ioread64(uctl_ctl_reg);
	uctl_ctl &= ~UCTL_CTL_H_CLKDIV_SEL;
	uctl_ctl |= FIELD_PREP(UCTL_CTL_H_CLKDIV_SEL, div);
	uctl_ctl |= UCTL_CTL_H_CLK_EN;
	iowrite64(uctl_ctl, uctl_ctl_reg);
	uctl_ctl = ioread64(uctl_ctl_reg);
	if (div != FIELD_GET(UCTL_CTL_H_CLKDIV_SEL, uctl_ctl) ||
	    !(uctl_ctl & UCTL_CTL_H_CLK_EN)) {
		printf("dwc3 controller clock init failure\n");
		return -EINVAL;
	}

	/* Step 4c: Deassert the controller clock divider reset. */
	uctl_ctl = ioread64(uctl_ctl_reg);
	uctl_ctl &= ~UCTL_CTL_H_CLKDIV_RST;
	iowrite64(uctl_ctl, uctl_ctl_reg);

	/* Step 5a: Reference clock configuration. */
	uctl_ctl = ioread64(uctl_ctl_reg);
	uctl_ctl &= ~UCTL_CTL_REF_CLK_SEL;
	uctl_ctl |= FIELD_PREP(UCTL_CTL_REF_CLK_SEL, ref_clk_sel);
	uctl_ctl &= ~UCTL_CTL_REF_CLK_FSEL;
	uctl_ctl |= FIELD_PREP(UCTL_CTL_REF_CLK_FSEL, 0x07);
	uctl_ctl &= ~UCTL_CTL_REF_CLK_DIV2;

	switch (clock_rate) {
	default:
		printf("Invalid ref_clk %u, using %u instead\n", CLOCK_100MHZ,
		       clock_rate);
		fallthrough;
	case CLOCK_100MHZ:
		mpll_mul = 0x19;
		if (ref_clk_sel < 2) {
			uctl_ctl &= ~UCTL_CTL_REF_CLK_FSEL;
			uctl_ctl |= FIELD_PREP(UCTL_CTL_REF_CLK_FSEL, 0x27);
		}
		break;
	case CLOCK_50MHZ:
		mpll_mul = 0x32;
		break;
	case CLOCK_125MHZ:
		mpll_mul = 0x28;
		break;
	}
	uctl_ctl &= ~UCTL_CTL_MPLL_MULTIPLIER;
	uctl_ctl |= FIELD_PREP(UCTL_CTL_MPLL_MULTIPLIER, mpll_mul);

	/* Step 5b: Configure and enable spread-spectrum for SuperSpeed. */
	uctl_ctl |= UCTL_CTL_SSC_EN;

	/* Step 5c: Enable SuperSpeed. */
	uctl_ctl |= UCTL_CTL_REF_SSP_EN;

	/* Step 5d: Configure PHYs. SKIP */

	/* Step 6a & 6b: Power up PHYs. */
	uctl_ctl |= UCTL_CTL_HS_POWER_EN;
	uctl_ctl |= UCTL_CTL_SS_POWER_EN;
	iowrite64(uctl_ctl, uctl_ctl_reg);

	/* Step 7: Wait 10 controller-clock cycles to take effect. */
	udelay(10);

	/* Step 8a: Deassert UCTL reset signal. */
	uctl_ctl = ioread64(uctl_ctl_reg);
	uctl_ctl &= ~UCTL_CTL_UCTL_RST;
	iowrite64(uctl_ctl, uctl_ctl_reg);

	/* Step 8b: Wait 10 controller-clock cycles. */
	udelay(10);

	/* Step 8c: Setup power-power control. */
	if (dwc3_octeon_config_power(dev, base)) {
		printf("Error configuring power\n");
		return -EINVAL;
	}

	/* Step 8d: Deassert UAHC reset signal. */
	uctl_ctl = ioread64(uctl_ctl_reg);
	uctl_ctl &= ~UCTL_CTL_UAHC_RST;
	iowrite64(uctl_ctl, uctl_ctl_reg);

	/* Step 8e: Wait 10 controller-clock cycles. */
	udelay(10);

	/* Step 9: Enable conditional coprocessor clock of UCTL. */
	uctl_ctl = ioread64(uctl_ctl_reg);
	uctl_ctl |= UCTL_CTL_SCLK_EN;
	iowrite64(uctl_ctl, uctl_ctl_reg);

	/* Step 10: Set for host mode only. */
	uctl_ctl = ioread64(uctl_ctl_reg);
	uctl_ctl &= ~UCTL_CTL_DRD_MODE;
	iowrite64(uctl_ctl, uctl_ctl_reg);

	return 0;
}

static void dwc3_octeon_set_endian_mode(void __iomem *base)
{
	u64 shim_cfg;

	shim_cfg = ioread64(base + UCTL_SHIM_CFG);
	shim_cfg &= ~UCTL_SHIM_CFG_CSR_ENDIAN_MODE;
	shim_cfg |= FIELD_PREP(UCTL_SHIM_CFG_CSR_ENDIAN_MODE, 1);
	shim_cfg &= ~UCTL_SHIM_CFG_DMA_ENDIAN_MODE;
	shim_cfg |= FIELD_PREP(UCTL_SHIM_CFG_DMA_ENDIAN_MODE, 1);
	iowrite64(shim_cfg, base + UCTL_SHIM_CFG);
}

static void dwc3_octeon_phy_reset(void __iomem *base)
{
	u64 uctl_ctl;

	uctl_ctl = ioread64(base);
	uctl_ctl &= ~UCTL_CTL_UPHY_RST;
	iowrite64(uctl_ctl, base);
}

static int octeon_dwc3_glue_probe(struct udevice *dev)
{
	void __iomem *base;

	base = dev_remap_addr(dev);
	if (IS_ERR(base))
		return PTR_ERR(base);

	dwc3_octeon_clocks_start(dev, base);
	dwc3_octeon_set_endian_mode(base);
	dwc3_octeon_phy_reset(base);

	return 0;
}

static int octeon_dwc3_glue_bind(struct udevice *dev)
{
	ofnode node, dwc3_node;

	/* Find snps,dwc3 node from subnode */
	dwc3_node = ofnode_null();
	ofnode_for_each_subnode(node, dev_ofnode(dev)) {
		if (ofnode_device_is_compatible(node, "snps,dwc3"))
			dwc3_node = node;
	}

	if (!ofnode_valid(dwc3_node)) {
		printf("Can't find dwc3 subnode for %s\n", dev->name);
		return -ENODEV;
	}

	return dm_scan_fdt_dev(dev);
}

static const struct udevice_id octeon_dwc3_glue_ids[] = {
	{ .compatible = "cavium,octeon-7130-usb-uctl" },
	{ }
};

U_BOOT_DRIVER(dwc3_octeon_glue) = {
	.name = "dwc3_octeon_glue",
	.id = UCLASS_NOP,
	.of_match = octeon_dwc3_glue_ids,
	.probe = octeon_dwc3_glue_probe,
	.bind = octeon_dwc3_glue_bind,
	.flags = DM_FLAG_ALLOC_PRIV_DMA,
};
