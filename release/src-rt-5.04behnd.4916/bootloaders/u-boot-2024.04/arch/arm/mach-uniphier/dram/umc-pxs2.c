// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015-2017 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * based on commit 21b6e480f92ccc38fe0502e3116411d6509d3bf2 of Diag by:
 * Copyright (C) 2015 Socionext Inc.
 */

#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/io.h>
#include <linux/printk.h>
#include <linux/sizes.h>
#include <asm/processor.h>
#include <time.h>

#include "../init.h"
#include "../soc-info.h"
#include "ddrmphy-regs.h"
#include "umc-regs.h"

#define DRAM_CH_NR	3

enum dram_freq {
	DRAM_FREQ_1866M,
	DRAM_FREQ_2133M,
	DRAM_FREQ_NR,
};

enum dram_size {
	DRAM_SZ_256M,
	DRAM_SZ_512M,
	DRAM_SZ_NR,
};

/* PHY */
static u32 ddrphy_pgcr2[DRAM_FREQ_NR] = {0x00FC7E5D, 0x00FC90AB};
static u32 ddrphy_ptr0[DRAM_FREQ_NR] = {0x0EA09205, 0x10C0A6C6};
static u32 ddrphy_ptr1[DRAM_FREQ_NR] = {0x0DAC041B, 0x0FA104B1};
static u32 ddrphy_ptr3[DRAM_FREQ_NR] = {0x15171e45, 0x18182357};
static u32 ddrphy_ptr4[DRAM_FREQ_NR] = {0x0e9ad8e9, 0x10b34157};
static u32 ddrphy_dtpr0[DRAM_FREQ_NR] = {0x35a00d88, 0x39e40e88};
static u32 ddrphy_dtpr1[DRAM_FREQ_NR] = {0x2288cc2c, 0x228a04d0};
static u32 ddrphy_dtpr2[DRAM_FREQ_NR] = {0x50005e00, 0x50006a00};
static u32 ddrphy_dtpr3[DRAM_FREQ_NR] = {0x0010cb49, 0x0010ec89};
static u32 ddrphy_mr0[DRAM_FREQ_NR] = {0x00000115, 0x00000125};
static u32 ddrphy_mr2[DRAM_FREQ_NR] = {0x000002a0, 0x000002a8};

/* dependent on package and board design */
static u32 ddrphy_acbdlr0[DRAM_CH_NR] = {0x0000000c, 0x0000000c, 0x00000009};

/* DDR multiPHY */
static inline int ddrphy_get_rank(int dx)
{
	return dx / 2;
}

static void ddrphy_fifo_reset(void __iomem *phy_base)
{
	u32 tmp;

	tmp = readl(phy_base + MPHY_PGCR0);
	tmp &= ~MPHY_PGCR0_PHYFRST;
	writel(tmp, phy_base + MPHY_PGCR0);

	udelay(1);

	tmp |= MPHY_PGCR0_PHYFRST;
	writel(tmp, phy_base + MPHY_PGCR0);

	udelay(1);
}

static void ddrphy_vt_ctrl(void __iomem *phy_base, int enable)
{
	u32 tmp;

	tmp = readl(phy_base + MPHY_PGCR1);

	if (enable)
		tmp &= ~MPHY_PGCR1_INHVT;
	else
		tmp |= MPHY_PGCR1_INHVT;

	writel(tmp, phy_base + MPHY_PGCR1);

	if (!enable) {
		while (!(readl(phy_base + MPHY_PGSR1) & MPHY_PGSR1_VTSTOP))
			cpu_relax();
	}
}

static void ddrphy_dqs_delay_fixup(void __iomem *phy_base, int nr_dx, int step)
{
	int dx;
	u32 lcdlr1, rdqsd;
	void __iomem *dx_base = phy_base + MPHY_DX_BASE;

	ddrphy_vt_ctrl(phy_base, 0);

	for (dx = 0; dx < nr_dx; dx++) {
		lcdlr1 = readl(dx_base + MPHY_DX_LCDLR1);
		rdqsd = (lcdlr1 >> 8) & 0xff;
		rdqsd = clamp(rdqsd + step, 0U, 0xffU);
		lcdlr1 = (lcdlr1 & ~(0xff << 8)) | (rdqsd << 8);
		writel(lcdlr1, dx_base + MPHY_DX_LCDLR1);
		readl(dx_base + MPHY_DX_LCDLR1); /* relax */
		dx_base += MPHY_DX_STRIDE;
	}

	ddrphy_vt_ctrl(phy_base, 1);
}

static int ddrphy_get_system_latency(void __iomem *phy_base, int width)
{
	void __iomem *dx_base = phy_base + MPHY_DX_BASE;
	const int nr_dx = width / 8;
	int dx, rank;
	u32 gtr;
	int dgsl, dgsl_min = INT_MAX, dgsl_max = 0;

	for (dx = 0; dx < nr_dx; dx++) {
		gtr = readl(dx_base + MPHY_DX_GTR);
		for (rank = 0; rank < 4; rank++) {
			dgsl = gtr & 0x7;
			/* if dgsl is zero, this rank was not trained. skip. */
			if (dgsl) {
				dgsl_min = min(dgsl_min, dgsl);
				dgsl_max = max(dgsl_max, dgsl);
			}
			gtr >>= 3;
		}
		dx_base += MPHY_DX_STRIDE;
	}

	if (dgsl_min != dgsl_max)
		pr_warn("DQS Gateing System Latencies are not all leveled.\n");

	return dgsl_max;
}

static void ddrphy_init(void __iomem *phy_base, enum dram_freq freq, int width,
			int ch)
{
	u32 tmp;
	void __iomem *zq_base, *dx_base;
	int zq, dx;
	int nr_dx;

	nr_dx = width / 8;

	writel(MPHY_PIR_ZCALBYP, phy_base + MPHY_PIR);
	/*
	 * Disable RGLVT bit (Read DQS Gating LCDL Delay VT Compensation)
	 * to avoid read error issue.
	 */
	writel(0x07d81e37, phy_base + MPHY_PGCR0);
	writel(0x0200c4e0, phy_base + MPHY_PGCR1);

	tmp = ddrphy_pgcr2[freq];
	if (width >= 32)
		tmp |= MPHY_PGCR2_DUALCHN | MPHY_PGCR2_ACPDDC;
	writel(tmp, phy_base + MPHY_PGCR2);

	writel(ddrphy_ptr0[freq], phy_base + MPHY_PTR0);
	writel(ddrphy_ptr1[freq], phy_base + MPHY_PTR1);
	writel(0x00083def, phy_base + MPHY_PTR2);
	writel(ddrphy_ptr3[freq], phy_base + MPHY_PTR3);
	writel(ddrphy_ptr4[freq], phy_base + MPHY_PTR4);

	writel(ddrphy_acbdlr0[ch], phy_base + MPHY_ACBDLR0);

	writel(0x55555555, phy_base + MPHY_ACIOCR1);
	writel(0x00000000, phy_base + MPHY_ACIOCR2);
	writel(0x55555555, phy_base + MPHY_ACIOCR3);
	writel(0x00000000, phy_base + MPHY_ACIOCR4);
	writel(0x00000055, phy_base + MPHY_ACIOCR5);
	writel(0x00181aa4, phy_base + MPHY_DXCCR);

	writel(0x0024641e, phy_base + MPHY_DSGCR);
	writel(0x0000040b, phy_base + MPHY_DCR);
	writel(ddrphy_dtpr0[freq], phy_base + MPHY_DTPR0);
	writel(ddrphy_dtpr1[freq], phy_base + MPHY_DTPR1);
	writel(ddrphy_dtpr2[freq], phy_base + MPHY_DTPR2);
	writel(ddrphy_dtpr3[freq], phy_base + MPHY_DTPR3);
	writel(ddrphy_mr0[freq], phy_base + MPHY_MR0);
	writel(0x00000006, phy_base + MPHY_MR1);
	writel(ddrphy_mr2[freq], phy_base + MPHY_MR2);
	writel(0x00000000, phy_base + MPHY_MR3);

	tmp = 0;
	for (dx = 0; dx < nr_dx; dx++)
		tmp |= BIT(MPHY_DTCR_RANKEN_SHIFT + ddrphy_get_rank(dx));
	writel(0x90003087 | tmp, phy_base + MPHY_DTCR);

	writel(0x00000000, phy_base + MPHY_DTAR0);
	writel(0x00000008, phy_base + MPHY_DTAR1);
	writel(0x00000010, phy_base + MPHY_DTAR2);
	writel(0x00000018, phy_base + MPHY_DTAR3);
	writel(0xdd22ee11, phy_base + MPHY_DTDR0);
	writel(0x7788bb44, phy_base + MPHY_DTDR1);

	/* impedance control settings */
	writel(0x04048900, phy_base + MPHY_ZQCR);

	zq_base = phy_base + MPHY_ZQ_BASE;
	for (zq = 0; zq < 4; zq++) {
		/*
		 * board-dependent
		 * PXS2: CH0ZQ0=0x5B, CH1ZQ0=0x5B, CH2ZQ0=0x59, others=0x5D
		 */
		writel(0x0007BB5D, zq_base + MPHY_ZQ_PR);
		zq_base += MPHY_ZQ_STRIDE;
	}

	/* DATX8 settings */
	dx_base = phy_base + MPHY_DX_BASE;
	for (dx = 0; dx < 4; dx++) {
		tmp = readl(dx_base + MPHY_DX_GCR0);
		tmp &= ~MPHY_DX_GCR0_WLRKEN_MASK;
		tmp |= BIT(MPHY_DX_GCR0_WLRKEN_SHIFT + ddrphy_get_rank(dx)) &
						MPHY_DX_GCR0_WLRKEN_MASK;
		writel(tmp, dx_base + MPHY_DX_GCR0);

		writel(0x00000000, dx_base + MPHY_DX_GCR1);
		writel(0x00000000, dx_base + MPHY_DX_GCR2);
		writel(0x00000000, dx_base + MPHY_DX_GCR3);
		dx_base += MPHY_DX_STRIDE;
	}

	while (!(readl(phy_base + MPHY_PGSR0) & MPHY_PGSR0_IDONE))
		cpu_relax();

	ddrphy_dqs_delay_fixup(phy_base, nr_dx, -4);
}

struct ddrphy_init_sequence {
	char *description;
	u32 init_flag;
	u32 done_flag;
	u32 err_flag;
};

static const struct ddrphy_init_sequence impedance_calibration_sequence[] = {
	{
		"Impedance Calibration",
		MPHY_PIR_ZCAL,
		MPHY_PGSR0_ZCDONE,
		MPHY_PGSR0_ZCERR,
	},
	{ /* sentinel */ }
};

static const struct ddrphy_init_sequence dram_init_sequence[] = {
	{
		"DRAM Initialization",
		MPHY_PIR_DRAMRST | MPHY_PIR_DRAMINIT,
		MPHY_PGSR0_DIDONE,
		0,
	},
	{ /* sentinel */ }
};

static const struct ddrphy_init_sequence training_sequence[] = {
	{
		"Write Leveling",
		MPHY_PIR_WL,
		MPHY_PGSR0_WLDONE,
		MPHY_PGSR0_WLERR,
	},
	{
		"Read DQS Gate Training",
		MPHY_PIR_QSGATE,
		MPHY_PGSR0_QSGDONE,
		MPHY_PGSR0_QSGERR,
	},
	{
		"Write Leveling Adjustment",
		MPHY_PIR_WLADJ,
		MPHY_PGSR0_WLADONE,
		MPHY_PGSR0_WLAERR,
	},
	{
		"Read Bit Deskew",
		MPHY_PIR_RDDSKW,
		MPHY_PGSR0_RDDONE,
		MPHY_PGSR0_RDERR,
	},
	{
		"Write Bit Deskew",
		MPHY_PIR_WRDSKW,
		MPHY_PGSR0_WDDONE,
		MPHY_PGSR0_WDERR,
	},
	{
		"Read Eye Training",
		MPHY_PIR_RDEYE,
		MPHY_PGSR0_REDONE,
		MPHY_PGSR0_REERR,
	},
	{
		"Write Eye Training",
		MPHY_PIR_WREYE,
		MPHY_PGSR0_WEDONE,
		MPHY_PGSR0_WEERR,
	},
	{ /* sentinel */ }
};

static int __ddrphy_training(void __iomem *phy_base,
			     const struct ddrphy_init_sequence *seq)
{
	const struct ddrphy_init_sequence *s;
	u32 pgsr0;
	u32 init_flag = MPHY_PIR_INIT;
	u32 done_flag = MPHY_PGSR0_IDONE;
	int timeout = 50000; /* 50 msec is long enough */
	unsigned long start = 0;

#ifdef DEBUG
	start = get_timer(0);
#endif

	for (s = seq; s->description; s++) {
		init_flag |= s->init_flag;
		done_flag |= s->done_flag;
	}

	writel(init_flag, phy_base + MPHY_PIR);

	do {
		if (--timeout < 0) {
			pr_err("%s: error: timeout during DDR training\n",
			       __func__);
			return -ETIMEDOUT;
		}
		udelay(1);
		pgsr0 = readl(phy_base + MPHY_PGSR0);
	} while ((pgsr0 & done_flag) != done_flag);

	for (s = seq; s->description; s++) {
		if (pgsr0 & s->err_flag) {
			pr_err("%s: error: %s failed\n", __func__,
			       s->description);
			return -EIO;
		}
	}

	pr_debug("DDRPHY training: elapsed time %ld msec\n", get_timer(start));

	return 0;
}

static int ddrphy_impedance_calibration(void __iomem *phy_base)
{
	int ret;
	u32 tmp;

	ret = __ddrphy_training(phy_base, impedance_calibration_sequence);
	if (ret)
		return ret;

	/*
	 * Because of a hardware bug, IDONE flag is set when the first ZQ block
	 * is calibrated.  The flag does not guarantee the completion for all
	 * the ZQ blocks.  Wait a little more just in case.
	 */
	udelay(1);

	/* reflect ZQ settings and enable average algorithm*/
	tmp = readl(phy_base + MPHY_ZQCR);
	tmp |= MPHY_ZQCR_FORCE_ZCAL_VT_UPDATE;
	writel(tmp, phy_base + MPHY_ZQCR);
	tmp &= ~MPHY_ZQCR_FORCE_ZCAL_VT_UPDATE;
	tmp |= MPHY_ZQCR_AVGEN;
	writel(tmp, phy_base + MPHY_ZQCR);

	return 0;
}

static int ddrphy_dram_init(void __iomem *phy_base)
{
	return __ddrphy_training(phy_base, dram_init_sequence);
}

static int ddrphy_training(void __iomem *phy_base)
{
	return __ddrphy_training(phy_base, training_sequence);
}

/* UMC */
static u32 umc_cmdctla[DRAM_FREQ_NR] = {0x66DD131D, 0x77EE1722};
/*
 * The ch2 is a different generation UMC core.
 * The register spec is different, unfortunately.
 */
static u32 umc_cmdctlb_ch01[DRAM_FREQ_NR] = {0x13E87C44, 0x18F88C44};
static u32 umc_cmdctlb_ch2[DRAM_FREQ_NR] = {0x19E8DC44, 0x1EF8EC44};
static u32 umc_spcctla[DRAM_FREQ_NR][DRAM_SZ_NR] = {
	{0x004A071D, 0x0078071D},
	{0x0055081E, 0x0089081E},
};

static u32 umc_spcctlb[] = {0x00FF000A, 0x00FF000B};
/* The ch2 is different for some reason only hardware guys know... */
static u32 umc_flowctla_ch01[] = {0x0800001E, 0x08000022};
static u32 umc_flowctla_ch2[] = {0x0800001E, 0x0800001E};

static void umc_set_system_latency(void __iomem *dc_base, int phy_latency)
{
	u32 val;
	int latency;

	val = readl(dc_base + UMC_RDATACTL_D0);
	latency = (val & UMC_RDATACTL_RADLTY_MASK) >> UMC_RDATACTL_RADLTY_SHIFT;
	latency += (val & UMC_RDATACTL_RAD2LTY_MASK) >>
						UMC_RDATACTL_RAD2LTY_SHIFT;
	/*
	 * UMC works at the half clock rate of the PHY.
	 * The LSB of latency is ignored
	 */
	latency += phy_latency & ~1;

	val &= ~(UMC_RDATACTL_RADLTY_MASK | UMC_RDATACTL_RAD2LTY_MASK);
	if (latency > 0xf) {
		val |= 0xf << UMC_RDATACTL_RADLTY_SHIFT;
		val |= (latency - 0xf) << UMC_RDATACTL_RAD2LTY_SHIFT;
	} else {
		val |= latency << UMC_RDATACTL_RADLTY_SHIFT;
	}

	writel(val, dc_base + UMC_RDATACTL_D0);
	writel(val, dc_base + UMC_RDATACTL_D1);

	readl(dc_base + UMC_RDATACTL_D1); /* relax */
}

/* enable/disable auto refresh */
static void umc_refresh_ctrl(void __iomem *dc_base, int enable)
{
	u32 tmp;

	tmp = readl(dc_base + UMC_SPCSETB);
	tmp &= ~UMC_SPCSETB_AREFMD_MASK;

	if (enable)
		tmp |= UMC_SPCSETB_AREFMD_ARB;
	else
		tmp |= UMC_SPCSETB_AREFMD_REG;

	writel(tmp, dc_base + UMC_SPCSETB);
	udelay(1);
}

static void umc_ud_init(void __iomem *umc_base, int ch)
{
	writel(0x00000003, umc_base + UMC_BITPERPIXELMODE_D0);

	if (ch == 2)
		writel(0x00000033, umc_base + UMC_PAIR1DOFF_D0);
}

static int umc_dc_init(void __iomem *dc_base, enum dram_freq freq,
		       unsigned long size, int width, int ch)
{
	enum dram_size size_e;
	int latency;
	u32 val;

	switch (size) {
	case 0:
		return 0;
	case SZ_256M:
		size_e = DRAM_SZ_256M;
		break;
	case SZ_512M:
		size_e = DRAM_SZ_512M;
		break;
	default:
		pr_err("unsupported DRAM size 0x%08lx (per 16bit) for ch%d\n",
		       size, ch);
		return -EINVAL;
	}

	writel(umc_cmdctla[freq], dc_base + UMC_CMDCTLA);

	writel(ch == 2 ? umc_cmdctlb_ch2[freq] : umc_cmdctlb_ch01[freq],
	       dc_base + UMC_CMDCTLB);

	writel(umc_spcctla[freq][size_e], dc_base + UMC_SPCCTLA);
	writel(umc_spcctlb[freq], dc_base + UMC_SPCCTLB);

	val = 0x000e000e;
	latency = 12;
	/* ES2 inserted one more FF to the logic. */
	if (uniphier_get_soc_model() >= 2)
		latency += 2;

	if (latency > 0xf) {
		val |= 0xf << UMC_RDATACTL_RADLTY_SHIFT;
		val |= (latency - 0xf) << UMC_RDATACTL_RAD2LTY_SHIFT;
	} else {
		val |= latency << UMC_RDATACTL_RADLTY_SHIFT;
	}

	writel(val, dc_base + UMC_RDATACTL_D0);
	if (width >= 32)
		writel(val, dc_base + UMC_RDATACTL_D1);

	writel(0x04060A02, dc_base + UMC_WDATACTL_D0);
	if (width >= 32)
		writel(0x04060A02, dc_base + UMC_WDATACTL_D1);
	writel(0x04000000, dc_base + UMC_DATASET);
	writel(0x00400020, dc_base + UMC_DCCGCTL);
	writel(0x00000084, dc_base + UMC_FLOWCTLG);
	writel(0x00000000, dc_base + UMC_ACSSETA);

	writel(ch == 2 ? umc_flowctla_ch2[freq] : umc_flowctla_ch01[freq],
	       dc_base + UMC_FLOWCTLA);

	writel(0x00004400, dc_base + UMC_FLOWCTLC);
	writel(0x200A0A00, dc_base + UMC_SPCSETB);
	writel(0x00000520, dc_base + UMC_DFICUPDCTLA);
	writel(0x0000000D, dc_base + UMC_RESPCTL);

	if (ch != 2) {
		writel(0x00202000, dc_base + UMC_FLOWCTLB);
		writel(0xFDBFFFFF, dc_base + UMC_FLOWCTLOB0);
		writel(0xFFFFFFFF, dc_base + UMC_FLOWCTLOB1);
		writel(0x00080700, dc_base + UMC_BSICMAPSET);
	} else {
		writel(0x00200000, dc_base + UMC_FLOWCTLB);
		writel(0x00000000, dc_base + UMC_BSICMAPSET);
	}

	writel(0x00000000, dc_base + UMC_ERRMASKA);
	writel(0x00000000, dc_base + UMC_ERRMASKB);

	return 0;
}

static int umc_ch_init(void __iomem *umc_ch_base, enum dram_freq freq,
		       unsigned long size, unsigned int width, int ch)
{
	void __iomem *dc_base = umc_ch_base + 0x00011000;
	void __iomem *phy_base = umc_ch_base + 0x00030000;
	int ret;

	writel(0x00000002, dc_base + UMC_INITSET);
	while (readl(dc_base + UMC_INITSTAT) & BIT(2))
		cpu_relax();

	/* deassert PHY reset signals */
	writel(UMC_DIOCTLA_CTL_NRST | UMC_DIOCTLA_CFG_NRST,
	       dc_base + UMC_DIOCTLA);

	ddrphy_init(phy_base, freq, width, ch);

	ret = ddrphy_impedance_calibration(phy_base);
	if (ret)
		return ret;

	ddrphy_dram_init(phy_base);
	if (ret)
		return ret;

	ret = umc_dc_init(dc_base, freq, size, width, ch);
	if (ret)
		return ret;

	umc_ud_init(umc_ch_base, ch);

	ret = ddrphy_training(phy_base);
	if (ret)
		return ret;

	udelay(1);

	/* match the system latency between UMC and PHY */
	umc_set_system_latency(dc_base,
			       ddrphy_get_system_latency(phy_base, width));

	udelay(1);

	/* stop auto refresh before clearing FIFO in PHY */
	umc_refresh_ctrl(dc_base, 0);
	ddrphy_fifo_reset(phy_base);
	umc_refresh_ctrl(dc_base, 1);

	udelay(10);

	return 0;
}

static void um_init(void __iomem *um_base)
{
	writel(0x000000ff, um_base + UMC_MBUS0);
	writel(0x000000ff, um_base + UMC_MBUS1);
	writel(0x000000ff, um_base + UMC_MBUS2);
	writel(0x000000ff, um_base + UMC_MBUS3);
}

int uniphier_pxs2_umc_init(const struct uniphier_board_data *bd)
{
	void __iomem *um_base = (void __iomem *)0x5b600000;
	void __iomem *umc_ch_base = (void __iomem *)0x5b800000;
	enum dram_freq freq;
	int ch, ret;

	switch (bd->dram_freq) {
	case 1866:
		freq = DRAM_FREQ_1866M;
		break;
	case 2133:
		freq = DRAM_FREQ_2133M;
		break;
	default:
		pr_err("unsupported DRAM frequency %d MHz\n", bd->dram_freq);
		return -EINVAL;
	}

	for (ch = 0; ch < DRAM_CH_NR; ch++) {
		unsigned long size = bd->dram_ch[ch].size;
		unsigned int width = bd->dram_ch[ch].width;

		if (size) {
			ret = umc_ch_init(umc_ch_base, freq,
					  size / (width / 16), width, ch);
			if (ret) {
				pr_err("failed to initialize UMC ch%d\n", ch);
				return ret;
			}
		}

		umc_ch_base += 0x00200000;
	}

	um_init(um_base);

	return 0;
}
