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

enum dram_freq {
	DRAM_FREQ_1333M,
	DRAM_FREQ_1600M,
	DRAM_FREQ_NR,
};

enum dram_size {
	DRAM_SZ_128M,
	DRAM_SZ_256M,
	DRAM_SZ_NR,
};

static u32 umc_cmdctla_plus[DRAM_FREQ_NR] = {0x45990b11, 0x36bb0f17};
static u32 umc_cmdctlb_plus[DRAM_FREQ_NR] = {0x16958924, 0x18c6aa24};
static u32 umc_spcctla[DRAM_FREQ_NR][DRAM_SZ_NR] = {
	{0x00240512, 0x00350512},
	{0x002b0617, 0x003f0617},
};
static u32 umc_spcctlb[DRAM_FREQ_NR] = {0x00ff0006, 0x00ff0008};
static u32 umc_rdatactl[DRAM_FREQ_NR] = {0x000a00ac, 0x000c00ae};

static int umc_get_rank(int ch)
{
	return ch;	/* ch0: rank0, ch1: rank1 for this SoC */
}

static void umc_start_ssif(void __iomem *ssif_base)
{
	writel(0x00000000, ssif_base + 0x0000b004);
	writel(0xffffffff, ssif_base + 0x0000c004);
	writel(0x000fffcf, ssif_base + 0x0000c008);
	writel(0x00000001, ssif_base + 0x0000b000);
	writel(0x00000001, ssif_base + 0x0000c000);
	writel(0x03010101, ssif_base + UMC_MDMCHSEL);
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

	writel(0x00000001, ssif_base + UMC_CPURST);
	writel(0x00000001, ssif_base + UMC_IDSRST);
	writel(0x00000001, ssif_base + UMC_IXMRST);
	writel(0x00000001, ssif_base + UMC_MDMRST);
	writel(0x00000001, ssif_base + UMC_MDDRST);
	writel(0x00000001, ssif_base + UMC_SIORST);
	writel(0x00000001, ssif_base + UMC_VIORST);
	writel(0x00000001, ssif_base + UMC_FRCRST);
	writel(0x00000001, ssif_base + UMC_RGLRST);
	writel(0x00000001, ssif_base + UMC_AIORST);
	writel(0x00000001, ssif_base + UMC_DMDRST);
}

static int umc_dramcont_init(void __iomem *dc_base, void __iomem *ca_base,
			     int freq, unsigned long size, bool ddr3plus)
{
	enum dram_freq freq_e;
	enum dram_size size_e;

	if (!ddr3plus) {
		pr_err("DDR3 standard is not supported\n");
		return -EINVAL;
	}

	switch (freq) {
	case 1333:
		freq_e = DRAM_FREQ_1333M;
		break;
	case 1600:
		freq_e = DRAM_FREQ_1600M;
		break;
	default:
		pr_err("unsupported DRAM frequency %d MHz\n", freq);
		return -EINVAL;
	}

	switch (size) {
	case 0:
		return 0;
	case SZ_128M:
		size_e = DRAM_SZ_128M;
		break;
	case SZ_256M:
		size_e = DRAM_SZ_256M;
		break;
	default:
		pr_err("unsupported DRAM size 0x%08lx\n", size);
		return -EINVAL;
	}

	writel(umc_cmdctla_plus[freq_e], dc_base + UMC_CMDCTLA);
	writel(umc_cmdctlb_plus[freq_e], dc_base + UMC_CMDCTLB);
	writel(umc_spcctla[freq_e][size_e], dc_base + UMC_SPCCTLA);
	writel(umc_spcctlb[freq_e], dc_base + UMC_SPCCTLB);
	writel(umc_rdatactl[freq_e], dc_base + UMC_RDATACTL_D0);
	writel(0x04060806, dc_base + UMC_WDATACTL_D0);
	writel(0x04a02000, dc_base + UMC_DATASET);
	writel(0x00000000, ca_base + 0x2300);
	writel(0x00400020, dc_base + UMC_DCCGCTL);
	writel(0x00000003, dc_base + 0x7000);
	writel(0x0000000f, dc_base + 0x8000);
	writel(0x000000c3, dc_base + 0x8004);
	writel(0x00000071, dc_base + 0x8008);
	writel(0x0000003b, dc_base + UMC_DICGCTLA);
	writel(0x020a0808, dc_base + UMC_DICGCTLB);
	writel(0x00000004, dc_base + UMC_FLOWCTLG);
	writel(0x80000201, ca_base + 0xc20);
	writel(0x0801e01e, dc_base + UMC_FLOWCTLA);
	writel(0x00200000, dc_base + UMC_FLOWCTLB);
	writel(0x00004444, dc_base + UMC_FLOWCTLC);
	writel(0x200a0a00, dc_base + UMC_SPCSETB);
	writel(0x00000000, dc_base + UMC_SPCSETD);
	writel(0x00000520, dc_base + UMC_DFICUPDCTLA);

	return 0;
}

static int umc_ch_init(void __iomem *dc_base, void __iomem *ca_base,
		       int freq, unsigned long size, bool ddr3plus, int ch)
{
	void __iomem *phy_base = dc_base + 0x00001000;
	int ret;

	writel(UMC_INITSET_INIT1EN, dc_base + UMC_INITSET);
	while (readl(dc_base + UMC_INITSTAT) & UMC_INITSTAT_INIT1ST)
		cpu_relax();

	writel(0x00000101, dc_base + UMC_DIOCTLA);

	ret = uniphier_ld4_ddrphy_init(phy_base, freq, ddr3plus);
	if (ret)
		return ret;

	ddrphy_prepare_training(phy_base, umc_get_rank(ch));
	ret = ddrphy_training(phy_base);
	if (ret)
		return ret;

	return umc_dramcont_init(dc_base, ca_base, freq, size, ddr3plus);
}

int uniphier_ld4_umc_init(const struct uniphier_board_data *bd)
{
	void __iomem *umc_base = (void __iomem *)0x5b800000;
	void __iomem *ca_base = umc_base + 0x00001000;
	void __iomem *dc_base = umc_base + 0x00400000;
	void __iomem *ssif_base = umc_base;
	int ch, ret;

	for (ch = 0; ch < DRAM_CH_NR; ch++) {
		ret = umc_ch_init(dc_base, ca_base, bd->dram_freq,
				  bd->dram_ch[ch].size,
				  !!(bd->flags & UNIPHIER_BD_DDR3PLUS), ch);
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
