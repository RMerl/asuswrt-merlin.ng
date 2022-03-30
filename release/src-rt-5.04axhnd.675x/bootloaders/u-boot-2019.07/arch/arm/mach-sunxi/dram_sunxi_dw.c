// SPDX-License-Identifier: GPL-2.0+
/*
 * sun8i H3 platform dram controller init
 *
 * (C) Copyright 2007-2015 Allwinner Technology Co.
 *                         Jerry Wang <wangflord@allwinnertech.com>
 * (C) Copyright 2015      Vishnu Patekar <vishnupatekar0510@gmail.com>
 * (C) Copyright 2015      Hans de Goede <hdegoede@redhat.com>
 * (C) Copyright 2015      Jens Kuske <jenskuske@gmail.com>
 */
#include <common.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/dram.h>
#include <asm/arch/cpu.h>
#include <linux/kconfig.h>

static void mctl_phy_init(u32 val)
{
	struct sunxi_mctl_ctl_reg * const mctl_ctl =
			(struct sunxi_mctl_ctl_reg *)SUNXI_DRAM_CTL0_BASE;

	writel(val | PIR_INIT, &mctl_ctl->pir);
	mctl_await_completion(&mctl_ctl->pgsr[0], PGSR_INIT_DONE, 0x1);
}

static void mctl_set_bit_delays(struct dram_para *para)
{
	struct sunxi_mctl_ctl_reg * const mctl_ctl =
			(struct sunxi_mctl_ctl_reg *)SUNXI_DRAM_CTL0_BASE;
	int i, j;

	clrbits_le32(&mctl_ctl->pgcr[0], 1 << 26);

	for (i = 0; i < NR_OF_BYTE_LANES; i++)
		for (j = 0; j < LINES_PER_BYTE_LANE; j++)
			writel(DXBDLR_WRITE_DELAY(para->dx_write_delays[i][j]) |
			       DXBDLR_READ_DELAY(para->dx_read_delays[i][j]),
			       &mctl_ctl->dx[i].bdlr[j]);

	for (i = 0; i < 31; i++)
		writel(ACBDLR_WRITE_DELAY(para->ac_delays[i]),
		       &mctl_ctl->acbdlr[i]);

#ifdef CONFIG_MACH_SUN8I_R40
	/* DQSn, DMn, DQn output enable bit delay */
	for (i = 0; i < 4; i++)
		writel(0x6 << 24, &mctl_ctl->dx[i].sdlr);
#endif

	setbits_le32(&mctl_ctl->pgcr[0], 1 << 26);
}

enum {
	MBUS_PORT_CPU           = 0,
	MBUS_PORT_GPU           = 1,
	MBUS_PORT_UNUSED	= 2,
	MBUS_PORT_DMA           = 3,
	MBUS_PORT_VE            = 4,
	MBUS_PORT_CSI           = 5,
	MBUS_PORT_NAND          = 6,
	MBUS_PORT_SS            = 7,
	MBUS_PORT_TS            = 8,
	MBUS_PORT_DI            = 9,
	MBUS_PORT_DE            = 10,
	MBUS_PORT_DE_CFD        = 11,
	MBUS_PORT_UNKNOWN1	= 12,
	MBUS_PORT_UNKNOWN2	= 13,
	MBUS_PORT_UNKNOWN3	= 14,
};

enum {
	MBUS_QOS_LOWEST = 0,
	MBUS_QOS_LOW,
	MBUS_QOS_HIGH,
	MBUS_QOS_HIGHEST
};

inline void mbus_configure_port(u8 port,
				bool bwlimit,
				bool priority,
				u8 qos,         /* MBUS_QOS_LOWEST .. MBUS_QOS_HIGEST */
				u8 waittime,    /* 0 .. 0xf */
				u8 acs,         /* 0 .. 0xff */
				u16 bwl0,       /* 0 .. 0xffff, bandwidth limit in MB/s */
				u16 bwl1,
				u16 bwl2)
{
	struct sunxi_mctl_com_reg * const mctl_com =
			(struct sunxi_mctl_com_reg *)SUNXI_DRAM_COM_BASE;

	const u32 cfg0 = ( (bwlimit ? (1 << 0) : 0)
			   | (priority ? (1 << 1) : 0)
			   | ((qos & 0x3) << 2)
			   | ((waittime & 0xf) << 4)
			   | ((acs & 0xff) << 8)
			   | (bwl0 << 16) );
	const u32 cfg1 = ((u32)bwl2 << 16) | (bwl1 & 0xffff);

	debug("MBUS port %d cfg0 %08x cfg1 %08x\n", port, cfg0, cfg1);
	writel(cfg0, &mctl_com->mcr[port][0]);
	writel(cfg1, &mctl_com->mcr[port][1]);
}

#define MBUS_CONF(port, bwlimit, qos, acs, bwl0, bwl1, bwl2)	\
	mbus_configure_port(MBUS_PORT_ ## port, bwlimit, false, \
			    MBUS_QOS_ ## qos, 0, acs, bwl0, bwl1, bwl2)

static void mctl_set_master_priority_h3(void)
{
	struct sunxi_mctl_com_reg * const mctl_com =
			(struct sunxi_mctl_com_reg *)SUNXI_DRAM_COM_BASE;

	/* enable bandwidth limit windows and set windows size 1us */
	writel((1 << 16) | (400 << 0), &mctl_com->bwcr);

	/* set cpu high priority */
	writel(0x00000001, &mctl_com->mapr);

	MBUS_CONF(   CPU,  true, HIGHEST, 0,  512,  256,  128);
	MBUS_CONF(   GPU,  true,    HIGH, 0, 1536, 1024,  256);
	MBUS_CONF(UNUSED,  true, HIGHEST, 0,  512,  256,   96);
	MBUS_CONF(   DMA,  true, HIGHEST, 0,  256,  128,   32);
	MBUS_CONF(    VE,  true,    HIGH, 0, 1792, 1600,  256);
	MBUS_CONF(   CSI,  true, HIGHEST, 0,  256,  128,   32);
	MBUS_CONF(  NAND,  true,    HIGH, 0,  256,  128,   64);
	MBUS_CONF(    SS,  true, HIGHEST, 0,  256,  128,   64);
	MBUS_CONF(    TS,  true, HIGHEST, 0,  256,  128,   64);
	MBUS_CONF(    DI,  true,    HIGH, 0, 1024,  256,   64);
	MBUS_CONF(    DE,  true, HIGHEST, 3, 8192, 6120, 1024);
	MBUS_CONF(DE_CFD,  true,    HIGH, 0, 1024,  288,   64);
}

static void mctl_set_master_priority_a64(void)
{
	struct sunxi_mctl_com_reg * const mctl_com =
			(struct sunxi_mctl_com_reg *)SUNXI_DRAM_COM_BASE;

	/* enable bandwidth limit windows and set windows size 1us */
	writel(399, &mctl_com->tmr);
	writel((1 << 16), &mctl_com->bwcr);

	/* Port 2 is reserved per Allwinner's linux-3.10 source, yet they
	 * initialise it */
	MBUS_CONF(   CPU,  true, HIGHEST, 0,  160,  100,   80);
	MBUS_CONF(   GPU, false,    HIGH, 0, 1536, 1400,  256);
	MBUS_CONF(UNUSED,  true, HIGHEST, 0,  512,  256,   96);
	MBUS_CONF(   DMA,  true,    HIGH, 0,  256,   80,  100);
	MBUS_CONF(    VE,  true,    HIGH, 0, 1792, 1600,  256);
	MBUS_CONF(   CSI,  true,    HIGH, 0,  256,  128,    0);
	MBUS_CONF(  NAND,  true,    HIGH, 0,  256,  128,   64);
	MBUS_CONF(    SS,  true, HIGHEST, 0,  256,  128,   64);
	MBUS_CONF(    TS,  true, HIGHEST, 0,  256,  128,   64);
	MBUS_CONF(    DI,  true,    HIGH, 0, 1024,  256,   64);
	MBUS_CONF(    DE,  true,    HIGH, 2, 8192, 6144, 2048);
	MBUS_CONF(DE_CFD,  true,    HIGH, 0, 1280,  144,   64);

	writel(0x81000004, &mctl_com->mdfs_bwlr[2]);
}

static void mctl_set_master_priority_h5(void)
{
	struct sunxi_mctl_com_reg * const mctl_com =
			(struct sunxi_mctl_com_reg *)SUNXI_DRAM_COM_BASE;

	/* enable bandwidth limit windows and set windows size 1us */
	writel(399, &mctl_com->tmr);
	writel((1 << 16), &mctl_com->bwcr);

	/* set cpu high priority */
	writel(0x00000001, &mctl_com->mapr);

	/* Port 2 is reserved per Allwinner's linux-3.10 source, yet
	 * they initialise it */
	MBUS_CONF(   CPU, true, HIGHEST, 0,  300,  260,  150);
	MBUS_CONF(   GPU, true, HIGHEST, 0,  600,  400,  200);
	MBUS_CONF(UNUSED, true, HIGHEST, 0,  512,  256,   96);
	MBUS_CONF(   DMA, true, HIGHEST, 0,  256,  128,   32);
	MBUS_CONF(    VE, true, HIGHEST, 0, 1900, 1500, 1000);
	MBUS_CONF(   CSI, true, HIGHEST, 0,  150,  120,  100);
	MBUS_CONF(  NAND, true,    HIGH, 0,  256,  128,   64);
	MBUS_CONF(    SS, true, HIGHEST, 0,  256,  128,   64);
	MBUS_CONF(    TS, true, HIGHEST, 0,  256,  128,   64);
	MBUS_CONF(    DI, true,    HIGH, 0, 1024,  256,   64);
	MBUS_CONF(    DE, true, HIGHEST, 3, 3400, 2400, 1024);
	MBUS_CONF(DE_CFD, true, HIGHEST, 0,  600,  400,  200);
}

static void mctl_set_master_priority_r40(void)
{
	struct sunxi_mctl_com_reg * const mctl_com =
			(struct sunxi_mctl_com_reg *)SUNXI_DRAM_COM_BASE;

	/* enable bandwidth limit windows and set windows size 1us */
	writel(399, &mctl_com->tmr);
	writel((1 << 16), &mctl_com->bwcr);

	/* set cpu high priority */
	writel(0x00000001, &mctl_com->mapr);

	/* Port 2 is reserved per Allwinner's linux-3.10 source, yet
	 * they initialise it */
	MBUS_CONF(     CPU, true, HIGHEST, 0,  300,  260,  150);
	MBUS_CONF(     GPU, true, HIGHEST, 0,  600,  400,  200);
	MBUS_CONF(  UNUSED, true, HIGHEST, 0,  512,  256,   96);
	MBUS_CONF(     DMA, true, HIGHEST, 0,  256,  128,   32);
	MBUS_CONF(      VE, true, HIGHEST, 0, 1900, 1500, 1000);
	MBUS_CONF(     CSI, true, HIGHEST, 0,  150,  120,  100);
	MBUS_CONF(    NAND, true,    HIGH, 0,  256,  128,   64);
	MBUS_CONF(      SS, true, HIGHEST, 0,  256,  128,   64);
	MBUS_CONF(      TS, true, HIGHEST, 0,  256,  128,   64);
	MBUS_CONF(      DI, true,    HIGH, 0, 1024,  256,   64);

	/*
	 * The port names are probably wrong, but no correct sources
	 * are available.
	 */
	MBUS_CONF(      DE, true,    HIGH, 0,  128,   48,    0);
	MBUS_CONF(  DE_CFD, true,    HIGH, 0,  384,  256,    0);
	MBUS_CONF(UNKNOWN1, true, HIGHEST, 0,  512,  384,  256);
	MBUS_CONF(UNKNOWN2, true, HIGHEST, 2, 8192, 6144, 1024);
	MBUS_CONF(UNKNOWN3, true,    HIGH, 0, 1280,  144,   64);
}

static void mctl_set_master_priority(uint16_t socid)
{
	switch (socid) {
	case SOCID_H3:
		mctl_set_master_priority_h3();
		return;
	case SOCID_A64:
		mctl_set_master_priority_a64();
		return;
	case SOCID_H5:
		mctl_set_master_priority_h5();
		return;
	case SOCID_R40:
		mctl_set_master_priority_r40();
		return;
	}
}

static u32 bin_to_mgray(int val)
{
	static const u8 lookup_table[32] = {
		0x00, 0x01, 0x02, 0x03, 0x06, 0x07, 0x04, 0x05,
		0x0c, 0x0d, 0x0e, 0x0f, 0x0a, 0x0b, 0x08, 0x09,
		0x18, 0x19, 0x1a, 0x1b, 0x1e, 0x1f, 0x1c, 0x1d,
		0x14, 0x15, 0x16, 0x17, 0x12, 0x13, 0x10, 0x11,
	};

	return lookup_table[clamp(val, 0, 31)];
}

static int mgray_to_bin(u32 val)
{
	static const u8 lookup_table[32] = {
		0x00, 0x01, 0x02, 0x03, 0x06, 0x07, 0x04, 0x05,
		0x0e, 0x0f, 0x0c, 0x0d, 0x08, 0x09, 0x0a, 0x0b,
		0x1e, 0x1f, 0x1c, 0x1d, 0x18, 0x19, 0x1a, 0x1b,
		0x10, 0x11, 0x12, 0x13, 0x16, 0x17, 0x14, 0x15,
	};

	return lookup_table[val & 0x1f];
}

static void mctl_h3_zq_calibration_quirk(struct dram_para *para)
{
	struct sunxi_mctl_ctl_reg * const mctl_ctl =
			(struct sunxi_mctl_ctl_reg *)SUNXI_DRAM_CTL0_BASE;
	int zq_count;

#if defined CONFIG_SUNXI_DRAM_DW_16BIT
	zq_count = 4;
#else
	zq_count = 6;
#endif

	if ((readl(SUNXI_SRAMC_BASE + 0x24) & 0xff) == 0 &&
	    (readl(SUNXI_SRAMC_BASE + 0xf0) & 0x1) == 0) {
		u32 reg_val;

		clrsetbits_le32(&mctl_ctl->zqcr, 0xffff,
				CONFIG_DRAM_ZQ & 0xffff);

		writel(PIR_CLRSR, &mctl_ctl->pir);
		mctl_phy_init(PIR_ZCAL);

		reg_val = readl(&mctl_ctl->zqdr[0]);
		reg_val &= (0x1f << 16) | (0x1f << 0);
		reg_val |= reg_val << 8;
		writel(reg_val, &mctl_ctl->zqdr[0]);

		reg_val = readl(&mctl_ctl->zqdr[1]);
		reg_val &= (0x1f << 16) | (0x1f << 0);
		reg_val |= reg_val << 8;
		writel(reg_val, &mctl_ctl->zqdr[1]);
		writel(reg_val, &mctl_ctl->zqdr[2]);
	} else {
		int i;
		u16 zq_val[6];
		u8 val;

		writel(0x0a0a0a0a, &mctl_ctl->zqdr[2]);

		for (i = 0; i < zq_count; i++) {
			u8 zq = (CONFIG_DRAM_ZQ >> (i * 4)) & 0xf;

			writel((zq << 20) | (zq << 16) | (zq << 12) |
					(zq << 8) | (zq << 4) | (zq << 0),
					&mctl_ctl->zqcr);

			writel(PIR_CLRSR, &mctl_ctl->pir);
			mctl_phy_init(PIR_ZCAL);

			zq_val[i] = readl(&mctl_ctl->zqdr[0]) & 0xff;
			writel(REPEAT_BYTE(zq_val[i]), &mctl_ctl->zqdr[2]);

			writel(PIR_CLRSR, &mctl_ctl->pir);
			mctl_phy_init(PIR_ZCAL);

			val = readl(&mctl_ctl->zqdr[0]) >> 24;
			zq_val[i] |= bin_to_mgray(mgray_to_bin(val) - 1) << 8;
		}

		writel((zq_val[1] << 16) | zq_val[0], &mctl_ctl->zqdr[0]);
		writel((zq_val[3] << 16) | zq_val[2], &mctl_ctl->zqdr[1]);
		if (zq_count > 4)
			writel((zq_val[5] << 16) | zq_val[4],
			       &mctl_ctl->zqdr[2]);
	}
}

static void mctl_set_cr(uint16_t socid, struct dram_para *para)
{
	struct sunxi_mctl_com_reg * const mctl_com =
			(struct sunxi_mctl_com_reg *)SUNXI_DRAM_COM_BASE;

	writel(MCTL_CR_BL8 | MCTL_CR_INTERLEAVED |
#if defined CONFIG_SUNXI_DRAM_DDR3
	       MCTL_CR_DDR3 | MCTL_CR_2T |
#elif defined CONFIG_SUNXI_DRAM_DDR2
	       MCTL_CR_DDR2 | MCTL_CR_2T |
#elif defined CONFIG_SUNXI_DRAM_LPDDR3
	       MCTL_CR_LPDDR3 | MCTL_CR_1T |
#else
#error Unsupported DRAM type!
#endif
	       (para->bank_bits == 3 ? MCTL_CR_EIGHT_BANKS : MCTL_CR_FOUR_BANKS) |
	       MCTL_CR_BUS_FULL_WIDTH(para->bus_full_width) |
	       (para->dual_rank ? MCTL_CR_DUAL_RANK : MCTL_CR_SINGLE_RANK) |
	       MCTL_CR_PAGE_SIZE(para->page_size) |
	       MCTL_CR_ROW_BITS(para->row_bits), &mctl_com->cr);

	if (socid == SOCID_R40) {
		if (para->dual_rank)
			panic("Dual rank memory not supported\n");

		/* Mux pin to A15 address line for single rank memory. */
		setbits_le32(&mctl_com->cr_r1, MCTL_CR_R1_MUX_A15);
	}
}

static void mctl_sys_init(uint16_t socid, struct dram_para *para)
{
	struct sunxi_ccm_reg * const ccm =
			(struct sunxi_ccm_reg *)SUNXI_CCM_BASE;
	struct sunxi_mctl_ctl_reg * const mctl_ctl =
			(struct sunxi_mctl_ctl_reg *)SUNXI_DRAM_CTL0_BASE;

	clrbits_le32(&ccm->mbus0_clk_cfg, MBUS_CLK_GATE);
	clrbits_le32(&ccm->mbus_reset, CCM_MBUS_RESET_RESET);
	clrbits_le32(&ccm->ahb_gate0, 1 << AHB_GATE_OFFSET_MCTL);
	clrbits_le32(&ccm->ahb_reset0_cfg, 1 << AHB_RESET_OFFSET_MCTL);
	clrbits_le32(&ccm->pll5_cfg, CCM_PLL5_CTRL_EN);
	if (socid == SOCID_A64 || socid == SOCID_R40)
		clrbits_le32(&ccm->pll11_cfg, CCM_PLL11_CTRL_EN);
	udelay(10);

	clrbits_le32(&ccm->dram_clk_cfg, CCM_DRAMCLK_CFG_RST);
	udelay(1000);

	if (socid == SOCID_A64 || socid == SOCID_R40) {
		clock_set_pll11(CONFIG_DRAM_CLK * 2 * 1000000, false);
		clrsetbits_le32(&ccm->dram_clk_cfg,
				CCM_DRAMCLK_CFG_DIV_MASK |
				CCM_DRAMCLK_CFG_SRC_MASK,
				CCM_DRAMCLK_CFG_DIV(1) |
				CCM_DRAMCLK_CFG_SRC_PLL11 |
				CCM_DRAMCLK_CFG_UPD);
	} else if (socid == SOCID_H3 || socid == SOCID_H5) {
		clock_set_pll5(CONFIG_DRAM_CLK * 2 * 1000000, false);
		clrsetbits_le32(&ccm->dram_clk_cfg,
				CCM_DRAMCLK_CFG_DIV_MASK |
				CCM_DRAMCLK_CFG_SRC_MASK,
				CCM_DRAMCLK_CFG_DIV(1) |
				CCM_DRAMCLK_CFG_SRC_PLL5 |
				CCM_DRAMCLK_CFG_UPD);
	}
	mctl_await_completion(&ccm->dram_clk_cfg, CCM_DRAMCLK_CFG_UPD, 0);

	setbits_le32(&ccm->ahb_reset0_cfg, 1 << AHB_RESET_OFFSET_MCTL);
	setbits_le32(&ccm->ahb_gate0, 1 << AHB_GATE_OFFSET_MCTL);
	setbits_le32(&ccm->mbus_reset, CCM_MBUS_RESET_RESET);
	setbits_le32(&ccm->mbus0_clk_cfg, MBUS_CLK_GATE);

	setbits_le32(&ccm->dram_clk_cfg, CCM_DRAMCLK_CFG_RST);
	udelay(10);

	writel(socid == SOCID_H5 ? 0x8000 : 0xc00e, &mctl_ctl->clken);
	udelay(500);
}

/* These are more guessed based on some Allwinner code. */
#define DX_GCR_ODT_DYNAMIC	(0x0 << 4)
#define DX_GCR_ODT_ALWAYS_ON	(0x1 << 4)
#define DX_GCR_ODT_OFF		(0x2 << 4)

static int mctl_channel_init(uint16_t socid, struct dram_para *para)
{
	struct sunxi_mctl_com_reg * const mctl_com =
			(struct sunxi_mctl_com_reg *)SUNXI_DRAM_COM_BASE;
	struct sunxi_mctl_ctl_reg * const mctl_ctl =
			(struct sunxi_mctl_ctl_reg *)SUNXI_DRAM_CTL0_BASE;

	unsigned int i;

	mctl_set_cr(socid, para);
	mctl_set_timing_params(socid, para);
	mctl_set_master_priority(socid);

	/* setting VTC, default disable all VT */
	clrbits_le32(&mctl_ctl->pgcr[0], (1 << 30) | 0x3f);
	if (socid == SOCID_H5)
		setbits_le32(&mctl_ctl->pgcr[1], (1 << 24) | (1 << 26));
	else
		clrsetbits_le32(&mctl_ctl->pgcr[1], 1 << 24, 1 << 26);

	/* increase DFI_PHY_UPD clock */
	writel(PROTECT_MAGIC, &mctl_com->protect);
	udelay(100);
	clrsetbits_le32(&mctl_ctl->upd2, 0xfff << 16, 0x50 << 16);
	writel(0x0, &mctl_com->protect);
	udelay(100);

	/* set dramc odt */
	for (i = 0; i < 4; i++) {
		u32 clearmask = (0x3 << 4) | (0x1 << 1) | (0x3 << 2) |
				(0x3 << 12) | (0x3 << 14);
		u32 setmask = IS_ENABLED(CONFIG_DRAM_ODT_EN) ?
				DX_GCR_ODT_DYNAMIC : DX_GCR_ODT_OFF;

		if (socid == SOCID_H5) {
			clearmask |= 0x2 << 8;
			setmask |= 0x4 << 8;
		}
		clrsetbits_le32(&mctl_ctl->dx[i].gcr, clearmask, setmask);
	}

	/* AC PDR should always ON */
	clrsetbits_le32(&mctl_ctl->aciocr, socid == SOCID_H5 ? (0x1 << 11) : 0,
			0x1 << 1);

	/* set DQS auto gating PD mode */
	setbits_le32(&mctl_ctl->pgcr[2], 0x3 << 6);

	if (socid == SOCID_H3) {
		/* dx ddr_clk & hdr_clk dynamic mode */
		clrbits_le32(&mctl_ctl->pgcr[0], (0x3 << 14) | (0x3 << 12));

		/* dphy & aphy phase select 270 degree */
		clrsetbits_le32(&mctl_ctl->pgcr[2], (0x3 << 10) | (0x3 << 8),
				(0x1 << 10) | (0x2 << 8));
	} else if (socid == SOCID_A64 || socid == SOCID_H5) {
		/* dphy & aphy phase select ? */
		clrsetbits_le32(&mctl_ctl->pgcr[2], (0x3 << 10) | (0x3 << 8),
				(0x0 << 10) | (0x3 << 8));
	} else if (socid == SOCID_R40) {
		/* dx ddr_clk & hdr_clk dynamic mode (tpr13[9] == 0) */
		clrbits_le32(&mctl_ctl->pgcr[0], (0x3 << 14) | (0x3 << 12));

		/* dphy & aphy phase select ? */
		clrsetbits_le32(&mctl_ctl->pgcr[2], (0x3 << 10) | (0x3 << 8),
				(0x0 << 10) | (0x3 << 8));
	}

	/* set half DQ */
	if (!para->bus_full_width) {
#if defined CONFIG_SUNXI_DRAM_DW_32BIT
		writel(0x0, &mctl_ctl->dx[2].gcr);
		writel(0x0, &mctl_ctl->dx[3].gcr);
#elif defined CONFIG_SUNXI_DRAM_DW_16BIT
		writel(0x0, &mctl_ctl->dx[1].gcr);
#else
#error Unsupported DRAM bus width!
#endif
	}

	/* data training configuration */
	clrsetbits_le32(&mctl_ctl->dtcr, 0xf << 24,
			(para->dual_rank ? 0x3 : 0x1) << 24);

	mctl_set_bit_delays(para);
	udelay(50);

	if (socid == SOCID_H3) {
		mctl_h3_zq_calibration_quirk(para);

		mctl_phy_init(PIR_PLLINIT | PIR_DCAL | PIR_PHYRST |
			      PIR_DRAMRST | PIR_DRAMINIT | PIR_QSGATE);
	} else if (socid == SOCID_A64 || socid == SOCID_H5) {
		clrsetbits_le32(&mctl_ctl->zqcr, 0xffffff, CONFIG_DRAM_ZQ);

		mctl_phy_init(PIR_ZCAL | PIR_PLLINIT | PIR_DCAL | PIR_PHYRST |
			      PIR_DRAMRST | PIR_DRAMINIT | PIR_QSGATE);
		/* no PIR_QSGATE for H5 ???? */
	} else if (socid == SOCID_R40) {
		clrsetbits_le32(&mctl_ctl->zqcr, 0xffffff, CONFIG_DRAM_ZQ);

		mctl_phy_init(PIR_ZCAL | PIR_PLLINIT | PIR_DCAL | PIR_PHYRST |
			      PIR_DRAMRST | PIR_DRAMINIT);
	}

	/* detect ranks and bus width */
	if (readl(&mctl_ctl->pgsr[0]) & (0xfe << 20)) {
		/* only one rank */
		if (((readl(&mctl_ctl->dx[0].gsr[0]) >> 24) & 0x2)
#if defined CONFIG_SUNXI_DRAM_DW_32BIT
		    || ((readl(&mctl_ctl->dx[1].gsr[0]) >> 24) & 0x2)
#endif
		    ) {
			clrsetbits_le32(&mctl_ctl->dtcr, 0xf << 24, 0x1 << 24);
			para->dual_rank = 0;
		}

		/* only half DQ width */
#if defined CONFIG_SUNXI_DRAM_DW_32BIT
		if (((readl(&mctl_ctl->dx[2].gsr[0]) >> 24) & 0x1) ||
		    ((readl(&mctl_ctl->dx[3].gsr[0]) >> 24) & 0x1)) {
			writel(0x0, &mctl_ctl->dx[2].gcr);
			writel(0x0, &mctl_ctl->dx[3].gcr);
			para->bus_full_width = 0;
		}
#elif defined CONFIG_SUNXI_DRAM_DW_16BIT
		if ((readl(&mctl_ctl->dx[1].gsr[0]) >> 24) & 0x1) {
			writel(0x0, &mctl_ctl->dx[1].gcr);
			para->bus_full_width = 0;
		}
#endif

		mctl_set_cr(socid, para);
		udelay(20);

		/* re-train */
		mctl_phy_init(PIR_QSGATE);
		if (readl(&mctl_ctl->pgsr[0]) & (0xfe << 20))
			return 1;
	}

	/* check the dramc status */
	mctl_await_completion(&mctl_ctl->statr, 0x1, 0x1);

	/* liuke added for refresh debug */
	setbits_le32(&mctl_ctl->rfshctl0, 0x1 << 31);
	udelay(10);
	clrbits_le32(&mctl_ctl->rfshctl0, 0x1 << 31);
	udelay(10);

	/* set PGCR3, CKE polarity */
	if (socid == SOCID_H3)
		writel(0x00aa0060, &mctl_ctl->pgcr[3]);
	else if (socid == SOCID_A64 || socid == SOCID_H5 || socid == SOCID_R40)
		writel(0xc0aa0060, &mctl_ctl->pgcr[3]);

	/* power down zq calibration module for power save */
	setbits_le32(&mctl_ctl->zqcr, ZQCR_PWRDOWN);

	/* enable master access */
	writel(0xffffffff, &mctl_com->maer);

	return 0;
}

static void mctl_auto_detect_dram_size(uint16_t socid, struct dram_para *para)
{
	/* detect row address bits */
	para->page_size = 512;
	para->row_bits = 16;
	para->bank_bits = 2;
	mctl_set_cr(socid, para);

	for (para->row_bits = 11; para->row_bits < 16; para->row_bits++)
		if (mctl_mem_matches((1 << (para->row_bits + para->bank_bits)) * para->page_size))
			break;

	/* detect bank address bits */
	para->bank_bits = 3;
	mctl_set_cr(socid, para);

	for (para->bank_bits = 2; para->bank_bits < 3; para->bank_bits++)
		if (mctl_mem_matches((1 << para->bank_bits) * para->page_size))
			break;

	/* detect page size */
	para->page_size = 8192;
	mctl_set_cr(socid, para);

	for (para->page_size = 512; para->page_size < 8192; para->page_size *= 2)
		if (mctl_mem_matches(para->page_size))
			break;
}

/*
 * The actual values used here are taken from Allwinner provided boot0
 * binaries, though they are probably board specific, so would likely benefit
 * from invidual tuning for each board. Apparently a lot of boards copy from
 * some Allwinner reference design, so we go with those generic values for now
 * in the hope that they are reasonable for most (all?) boards.
 */
#define SUN8I_H3_DX_READ_DELAYS					\
	{{ 18, 18, 18, 18, 18, 18, 18, 18, 18,  0,  0 },	\
	 { 14, 14, 14, 14, 14, 14, 14, 14, 14,  0,  0 },	\
	 { 18, 18, 18, 18, 18, 18, 18, 18, 18,  0,  0 },	\
	 { 14, 14, 14, 14, 14, 14, 14, 14, 14,  0,  0 }}
#define SUN8I_H3_DX_WRITE_DELAYS				\
	{{  0,  0,  0,  0,  0,  0,  0,  0,  0, 10, 10 },	\
	 {  0,  0,  0,  0,  0,  0,  0,  0,  0, 10, 10 },	\
	 {  0,  0,  0,  0,  0,  0,  0,  0,  0, 10, 10 },	\
	 {  0,  0,  0,  0,  0,  0,  0,  0,  0,  6,  6 }}
#define SUN8I_H3_AC_DELAYS					\
	{  0,  0,  0,  0,  0,  0,  0,  0,			\
	   0,  0,  0,  0,  0,  0,  0,  0,			\
	   0,  0,  0,  0,  0,  0,  0,  0,			\
	   0,  0,  0,  0,  0,  0,  0      }

#define SUN8I_R40_DX_READ_DELAYS				\
	{{ 14, 14, 14, 14, 14, 14, 14, 14, 14,  0,  0 },	\
	 { 14, 14, 14, 14, 14, 14, 14, 14, 14,  0,  0 },	\
	 { 14, 14, 14, 14, 14, 14, 14, 14, 14,  0,  0 },	\
	 { 14, 14, 14, 14, 14, 14, 14, 14, 14,  0,  0 } }
#define SUN8I_R40_DX_WRITE_DELAYS				\
	{{  0,  0,  0,  0,  0,  0,  0,  0,  0,  6,  0 },	\
	 {  0,  0,  0,  0,  0,  0,  0,  0,  0,  6,  0 },	\
	 {  0,  0,  0,  0,  0,  0,  0,  0,  0,  6,  0 },	\
	 {  0,  0,  0,  0,  0,  0,  0,  0,  0,  6,  0 } }
#define SUN8I_R40_AC_DELAYS					\
	{  0,  0,  3,  0,  0,  0,  0,  0,			\
	   0,  0,  0,  0,  0,  0,  0,  0,			\
	   0,  0,  0,  0,  0,  0,  0,  0,			\
	   0,  0,  0,  0,  0,  0,  0      }

#define SUN50I_A64_DX_READ_DELAYS				\
	{{ 16, 16, 16, 16, 17, 16, 16, 17, 16,  1,  0 },	\
	 { 17, 17, 17, 17, 17, 17, 17, 17, 17,  1,  0 },	\
	 { 16, 17, 17, 16, 16, 16, 16, 16, 16,  0,  0 },	\
	 { 17, 17, 17, 17, 17, 17, 17, 17, 17,  1,  0 }}
#define SUN50I_A64_DX_WRITE_DELAYS				\
	{{  0,  0,  0,  0,  0,  0,  0,  0,  0, 15, 15 },	\
	 {  0,  0,  0,  0,  1,  1,  1,  1,  0, 10, 10 },	\
	 {  1,  0,  1,  1,  1,  1,  1,  1,  0, 11, 11 },	\
	 {  1,  0,  0,  1,  1,  1,  1,  1,  0, 12, 12 }}
#define SUN50I_A64_AC_DELAYS					\
	{  5,  5, 13, 10,  2,  5,  3,  3,			\
	   0,  3,  3,  3,  1,  0,  0,  0,			\
	   3,  4,  0,  3,  4,  1,  4,  0,			\
	   1,  1,  0,  1, 13,  5,  4      }

#define SUN8I_H5_DX_READ_DELAYS					\
	{{ 14, 15, 17, 17, 17, 17, 17, 18, 17,  3,  3 },	\
	 { 21, 21, 12, 22, 21, 21, 21, 21, 21,  3,  3 },	\
	 { 16, 19, 19, 17, 22, 22, 21, 22, 19,  3,  3 },	\
	 { 21, 21, 22, 22, 20, 21, 19, 19, 19,  3,  3 } }
#define SUN8I_H5_DX_WRITE_DELAYS				\
	{{  1,  2,  3,  4,  3,  4,  4,  4,  6,  6,  6 },	\
	 {  6,  6,  6,  5,  5,  5,  5,  5,  6,  6,  6 },	\
	 {  0,  2,  4,  2,  6,  5,  5,  5,  6,  6,  6 },	\
	 {  3,  3,  3,  2,  2,  1,  1,  1,  4,  4,  4 } }
#define SUN8I_H5_AC_DELAYS					\
	{  0,  0,  5,  5,  0,  0,  0,  0,			\
	   0,  0,  0,  0,  3,  3,  3,  3,			\
	   3,  3,  3,  3,  3,  3,  3,  3,			\
	   3,  3,  3,  3,  2,  0,  0      }

unsigned long sunxi_dram_init(void)
{
	struct sunxi_mctl_com_reg * const mctl_com =
			(struct sunxi_mctl_com_reg *)SUNXI_DRAM_COM_BASE;
	struct sunxi_mctl_ctl_reg * const mctl_ctl =
			(struct sunxi_mctl_ctl_reg *)SUNXI_DRAM_CTL0_BASE;

	struct dram_para para = {
		.dual_rank = 1,
		.bus_full_width = 1,
		.row_bits = 15,
		.bank_bits = 3,
		.page_size = 4096,

#if defined(CONFIG_MACH_SUN8I_H3)
		.dx_read_delays  = SUN8I_H3_DX_READ_DELAYS,
		.dx_write_delays = SUN8I_H3_DX_WRITE_DELAYS,
		.ac_delays	 = SUN8I_H3_AC_DELAYS,
#elif defined(CONFIG_MACH_SUN8I_R40)
		.dx_read_delays  = SUN8I_R40_DX_READ_DELAYS,
		.dx_write_delays = SUN8I_R40_DX_WRITE_DELAYS,
		.ac_delays	 = SUN8I_R40_AC_DELAYS,
#elif defined(CONFIG_MACH_SUN50I)
		.dx_read_delays  = SUN50I_A64_DX_READ_DELAYS,
		.dx_write_delays = SUN50I_A64_DX_WRITE_DELAYS,
		.ac_delays	 = SUN50I_A64_AC_DELAYS,
#elif defined(CONFIG_MACH_SUN50I_H5)
		.dx_read_delays  = SUN8I_H5_DX_READ_DELAYS,
		.dx_write_delays = SUN8I_H5_DX_WRITE_DELAYS,
		.ac_delays	 = SUN8I_H5_AC_DELAYS,
#endif
	};
/*
 * Let the compiler optimize alternatives away by passing this value into
 * the static functions. This saves us #ifdefs, but still keeps the binary
 * small.
 */
#if defined(CONFIG_MACH_SUN8I_H3)
	uint16_t socid = SOCID_H3;
#elif defined(CONFIG_MACH_SUN8I_R40)
	uint16_t socid = SOCID_R40;
	/* Currently we cannot support R40 with dual rank memory */
	para.dual_rank = 0;
#elif defined(CONFIG_MACH_SUN8I_V3S)
	/* TODO: set delays and mbus priority for V3s */
	uint16_t socid = SOCID_H3;
#elif defined(CONFIG_MACH_SUN50I)
	uint16_t socid = SOCID_A64;
#elif defined(CONFIG_MACH_SUN50I_H5)
	uint16_t socid = SOCID_H5;
#endif

	mctl_sys_init(socid, &para);
	if (mctl_channel_init(socid, &para))
		return 0;

	if (para.dual_rank)
		writel(0x00000303, &mctl_ctl->odtmap);
	else
		writel(0x00000201, &mctl_ctl->odtmap);
	udelay(1);

	/* odt delay */
	if (socid == SOCID_H3)
		writel(0x0c000400, &mctl_ctl->odtcfg);

	if (socid == SOCID_A64 || socid == SOCID_H5 || socid == SOCID_R40) {
		/* VTF enable (tpr13[8] == 1) */
		setbits_le32(&mctl_ctl->vtfcr,
			     (socid != SOCID_A64 ? 3 : 2) << 8);
		/* DQ hold disable (tpr13[26] == 1) */
		clrbits_le32(&mctl_ctl->pgcr[2], (1 << 13));
	}

	/* clear credit value */
	setbits_le32(&mctl_com->cccr, 1 << 31);
	udelay(10);

	mctl_auto_detect_dram_size(socid, &para);
	mctl_set_cr(socid, &para);

	return (1UL << (para.row_bits + para.bank_bits)) * para.page_size *
	       (para.dual_rank ? 2 : 1);
}
