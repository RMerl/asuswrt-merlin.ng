// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2011-2014 Panasonic Corporation
 * Copyright (C) 2015-2016 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#include <common.h>
#include <linux/errno.h>
#include <linux/io.h>
#include <linux/sizes.h>
#include <asm/processor.h>

#include "../init.h"
#include "ddrphy-init.h"
#include "umc-regs.h"

#define DRAM_CH_NR	2

enum dram_size {
	DRAM_SZ_128M,
	DRAM_SZ_256M,
	DRAM_SZ_512M,
	DRAM_SZ_NR,
};

static u32 umc_spcctla[DRAM_SZ_NR] = {0x002b0617, 0x003f0617, 0x00770617};

static void umc_start_ssif(void __iomem *ssif_base)
{
	writel(0x00000000, ssif_base + 0x0000b004);
	writel(0xffffffff, ssif_base + 0x0000c004);
	writel(0x000fffcf, ssif_base + 0x0000c008);
	writel(0x00000001, ssif_base + 0x0000b000);
	writel(0x00000001, ssif_base + 0x0000c000);

	writel(0x03010100, ssif_base + UMC_HDMCHSEL);
	writel(0x03010101, ssif_base + UMC_MDMCHSEL);
	writel(0x03010100, ssif_base + UMC_DVCCHSEL);
	writel(0x03010100, ssif_base + UMC_DMDCHSEL);

	writel(0x00000000, ssif_base + UMC_CLKEN_SSIF_FETCH);
	writel(0x00000000, ssif_base + UMC_CLKEN_SSIF_COMQUE0);
	writel(0x00000000, ssif_base + UMC_CLKEN_SSIF_COMWC0);
	writel(0x00000000, ssif_base + UMC_CLKEN_SSIF_COMRC0);
	writel(0x00000000, ssif_base + UMC_CLKEN_SSIF_COMQUE1);
	writel(0x00000000, ssif_base + UMC_CLKEN_SSIF_COMWC1);
	writel(0x00000000, ssif_base + UMC_CLKEN_SSIF_COMRC1);
	writel(0x00000000, ssif_base + UMC_CLKEN_SSIF_WC);
	writel(0x00000000, ssif_base + UMC_CLKEN_SSIF_RC);
	writel(0x00000000, ssif_base + UMC_CLKEN_SSIF_DST);
	writel(0x00000000, ssif_base + 0x0000c044);		/* DCGIV_SSIF_REG */

	writel(0x00000001, ssif_base + UMC_CPURST);
	writel(0x00000001, ssif_base + UMC_IDSRST);
	writel(0x00000001, ssif_base + UMC_IXMRST);
	writel(0x00000001, ssif_base + UMC_HDMRST);
	writel(0x00000001, ssif_base + UMC_MDMRST);
	writel(0x00000001, ssif_base + UMC_HDDRST);
	writel(0x00000001, ssif_base + UMC_MDDRST);
	writel(0x00000001, ssif_base + UMC_SIORST);
	writel(0x00000001, ssif_base + UMC_GIORST);
	writel(0x00000001, ssif_base + UMC_HD2RST);
	writel(0x00000001, ssif_base + UMC_VIORST);
	writel(0x00000001, ssif_base + UMC_DVCRST);
	writel(0x00000001, ssif_base + UMC_RGLRST);
	writel(0x00000001, ssif_base + UMC_VPERST);
	writel(0x00000001, ssif_base + UMC_AIORST);
	writel(0x00000001, ssif_base + UMC_DMDRST);
}

static int umc_dramcont_init(void __iomem *dc_base, void __iomem *ca_base,
			     int freq, unsigned long size, bool ddr3plus)
{
	enum dram_size size_e;

	if (freq != 1600) {
		pr_err("Unsupported DDR frequency %d MHz\n", freq);
		return -EINVAL;
	}

	if (ddr3plus) {
		pr_err("DDR3+ is not supported\n");
		return -EINVAL;
	}

	switch (size) {
	case SZ_128M:
		size_e = DRAM_SZ_128M;
		break;
	case SZ_256M:
		size_e = DRAM_SZ_256M;
		break;
	case SZ_512M:
		size_e = DRAM_SZ_512M;
		break;
	default:
		pr_err("unsupported DRAM size 0x%08lx (per 16bit)\n", size);
		return -EINVAL;
	}

	writel(0x66bb0f17, dc_base + UMC_CMDCTLA);
	writel(0x18c6aa44, dc_base + UMC_CMDCTLB);
	writel(umc_spcctla[size_e], dc_base + UMC_SPCCTLA);
	writel(0x00ff0008, dc_base + UMC_SPCCTLB);
	writel(0x000c00ae, dc_base + UMC_RDATACTL_D0);
	writel(0x000c00ae, dc_base + UMC_RDATACTL_D1);
	writel(0x04060802, dc_base + UMC_WDATACTL_D0);
	writel(0x04060802, dc_base + UMC_WDATACTL_D1);
	writel(0x04a02000, dc_base + UMC_DATASET);
	writel(0x00000000, ca_base + 0x2300);
	writel(0x00400020, dc_base + UMC_DCCGCTL);
	writel(0x0000000f, dc_base + 0x7000);
	writel(0x0000000f, dc_base + 0x8000);
	writel(0x000000c3, dc_base + 0x8004);
	writel(0x00000071, dc_base + 0x8008);
	writel(0x00000004, dc_base + UMC_FLOWCTLG);
	writel(0x00000000, dc_base + 0x0060);
	writel(0x80000201, ca_base + 0xc20);
	writel(0x0801e01e, dc_base + UMC_FLOWCTLA);
	writel(0x00200000, dc_base + UMC_FLOWCTLB);
	writel(0x00004444, dc_base + UMC_FLOWCTLC);
	writel(0x200a0a00, dc_base + UMC_SPCSETB);
	writel(0x00010000, dc_base + UMC_SPCSETD);
	writel(0x80000020, dc_base + UMC_DFICUPDCTLA);

	return 0;
}

static int umc_ch_init(void __iomem *dc_base, void __iomem *ca_base,
		       int freq, unsigned long size, unsigned int width,
		       bool ddr3plus)
{
	void __iomem *phy_base = dc_base + 0x00001000;
	int nr_phy = width / 16;
	int phy, ret;

	writel(UMC_INITSET_INIT1EN, dc_base + UMC_INITSET);
	while (readl(dc_base + UMC_INITSTAT) & UMC_INITSTAT_INIT1ST)
		cpu_relax();

	for (phy = 0; phy < nr_phy; phy++) {
		writel(0x00000100 | ((1 << (phy + 1)) - 1),
		       dc_base + UMC_DIOCTLA);

		ret = uniphier_ld4_ddrphy_init(phy_base, freq, ddr3plus);
		if (ret)
			return ret;

		ddrphy_prepare_training(phy_base, phy);
		ret = ddrphy_training(phy_base);
		if (ret)
			return ret;

		phy_base += 0x00001000;
	}

	return umc_dramcont_init(dc_base, ca_base, freq, size / (width / 16),
				 ddr3plus);
}

int uniphier_pro4_umc_init(const struct uniphier_board_data *bd)
{
	void __iomem *umc_base = (void __iomem *)0x5b800000;
	void __iomem *ca_base = umc_base + 0x00001000;
	void __iomem *dc_base = umc_base + 0x00400000;
	void __iomem *ssif_base = umc_base;
	int ch, ret;

	for (ch = 0; ch < DRAM_CH_NR; ch++) {
		ret = umc_ch_init(dc_base, ca_base, bd->dram_freq,
				  bd->dram_ch[ch].size,
				  bd->dram_ch[ch].width,
				  !!(bd->flags & UNIPHIER_BD_DDR3PLUS));
		if (ret) {
			pr_err("failed to initialize UMC ch%d\n", ch);
			return ret;
		}

		ca_base += 0x00001000;
		dc_base += 0x00200000;
	}

	umc_start_ssif(ssif_base);

	return 0;
}
