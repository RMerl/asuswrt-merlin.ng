// SPDX-License-Identifier: GPL-2.0+
/*
 * TI serdes driver for keystone2.
 *
 * (C) Copyright 2014
 *     Texas Instruments Incorporated, <www.ti.com>
 */

#include <errno.h>
#include <common.h>
#include <asm/ti-common/keystone_serdes.h>

#define SERDES_CMU_REGS(x)		(0x0000 + (0x0c00 * (x)))
#define SERDES_LANE_REGS(x)		(0x0200 + (0x200 * (x)))
#define SERDES_COMLANE_REGS		0x0a00
#define SERDES_WIZ_REGS			0x1fc0

#define SERDES_CMU_REG_000(x)		(SERDES_CMU_REGS(x) + 0x000)
#define SERDES_CMU_REG_010(x)		(SERDES_CMU_REGS(x) + 0x010)
#define SERDES_COMLANE_REG_000		(SERDES_COMLANE_REGS + 0x000)
#define SERDES_LANE_REG_000(x)		(SERDES_LANE_REGS(x) + 0x000)
#define SERDES_LANE_REG_028(x)		(SERDES_LANE_REGS(x) + 0x028)
#define SERDES_LANE_CTL_STATUS_REG(x)	(SERDES_WIZ_REGS + 0x0020 + (4 * (x)))
#define SERDES_PLL_CTL_REG		(SERDES_WIZ_REGS + 0x0034)

#define SERDES_RESET			BIT(28)
#define SERDES_LANE_RESET		BIT(29)
#define SERDES_LANE_LOOPBACK		BIT(30)
#define SERDES_LANE_EN_VAL(x, y, z)	(x[y] | (z << 26) | (z << 10))

#define SERDES_CMU_CFG_NUM		5
#define SERDES_COMLANE_CFG_NUM		10
#define SERDES_LANE_CFG_NUM		10

struct serdes_cfg {
	u32 ofs;
	u32 val;
	u32 mask;
};

struct cfg_entry {
	enum ks2_serdes_clock clk;
	enum ks2_serdes_rate rate;
	struct serdes_cfg cmu[SERDES_CMU_CFG_NUM];
	struct serdes_cfg comlane[SERDES_COMLANE_CFG_NUM];
	struct serdes_cfg lane[SERDES_LANE_CFG_NUM];
};

/* SERDES PHY lane enable configuration value, indexed by PHY interface */
static u32 serdes_cfg_lane_enable[] = {
	0xf000f0c0,     /* SGMII */
	0xf0e9f038,     /* PCSR */
};

/* SERDES PHY PLL enable configuration value, indexed by PHY interface  */
static u32 serdes_cfg_pll_enable[] = {
	0xe0000000,     /* SGMII */
	0xee000000,     /* PCSR */
};

/**
 * Array to hold all possible serdes configurations.
 * Combination for 5 clock settings and 6 baud rates.
 */
static struct cfg_entry cfgs[] = {
	{
		.clk = SERDES_CLOCK_156P25M,
		.rate = SERDES_RATE_5G,
		.cmu = {
			{0x0000, 0x00800000, 0xffff0000},
			{0x0014, 0x00008282, 0x0000ffff},
			{0x0060, 0x00142438, 0x00ffffff},
			{0x0064, 0x00c3c700, 0x00ffff00},
			{0x0078, 0x0000c000, 0x0000ff00}
		},
		.comlane = {
			{0x0a00, 0x00000800, 0x0000ff00},
			{0x0a08, 0x38a20000, 0xffff0000},
			{0x0a30, 0x008a8a00, 0x00ffff00},
			{0x0a84, 0x00000600, 0x0000ff00},
			{0x0a94, 0x10000000, 0xff000000},
			{0x0aa0, 0x81000000, 0xff000000},
			{0x0abc, 0xff000000, 0xff000000},
			{0x0ac0, 0x0000008b, 0x000000ff},
			{0x0b08, 0x583f0000, 0xffff0000},
			{0x0b0c, 0x0000004e, 0x000000ff}
		},
		.lane = {
			{0x0004, 0x38000080, 0xff0000ff},
			{0x0008, 0x00000000, 0x000000ff},
			{0x000c, 0x02000000, 0xff000000},
			{0x0010, 0x1b000000, 0xff000000},
			{0x0014, 0x00006fb8, 0x0000ffff},
			{0x0018, 0x758000e4, 0xffff00ff},
			{0x00ac, 0x00004400, 0x0000ff00},
			{0x002c, 0x00100800, 0x00ffff00},
			{0x0080, 0x00820082, 0x00ff00ff},
			{0x0084, 0x1d0f0385, 0xffffffff}
		},
	},
};

static inline void ks2_serdes_rmw(u32 addr, u32 value, u32 mask)
{
	writel(((readl(addr) & (~mask)) | (value & mask)), addr);
}

static void ks2_serdes_cfg_setup(u32 base, struct serdes_cfg *cfg, u32 size)
{
	u32 i;

	for (i = 0; i < size; i++)
		ks2_serdes_rmw(base + cfg[i].ofs, cfg[i].val, cfg[i].mask);
}

static void ks2_serdes_lane_config(u32 base, struct serdes_cfg *cfg_lane,
				   u32 size, u32 lane)
{
	u32 i;

	for (i = 0; i < size; i++)
		ks2_serdes_rmw(base + cfg_lane[i].ofs + SERDES_LANE_REGS(lane),
			       cfg_lane[i].val, cfg_lane[i].mask);
}

static int ks2_serdes_init_cfg(u32 base, struct cfg_entry *cfg, u32 num_lanes)
{
	u32 i;

	ks2_serdes_cfg_setup(base, cfg->cmu, SERDES_CMU_CFG_NUM);
	ks2_serdes_cfg_setup(base, cfg->comlane, SERDES_COMLANE_CFG_NUM);

	for (i = 0; i < num_lanes; i++)
		ks2_serdes_lane_config(base, cfg->lane, SERDES_LANE_CFG_NUM, i);

	return 0;
}

static void ks2_serdes_cmu_comlane_enable(u32 base, struct ks2_serdes *serdes)
{
	/* Bring SerDes out of Reset */
	ks2_serdes_rmw(base + SERDES_CMU_REG_010(0), 0x0, SERDES_RESET);
	if (serdes->intf == SERDES_PHY_PCSR)
		ks2_serdes_rmw(base + SERDES_CMU_REG_010(1), 0x0, SERDES_RESET);

	/* Enable CMU and COMLANE */
	ks2_serdes_rmw(base + SERDES_CMU_REG_000(0), 0x03, 0x000000ff);
	if (serdes->intf == SERDES_PHY_PCSR)
		ks2_serdes_rmw(base + SERDES_CMU_REG_000(1), 0x03, 0x000000ff);

	ks2_serdes_rmw(base + SERDES_COMLANE_REG_000, 0x5f, 0x000000ff);
}

static void ks2_serdes_pll_enable(u32 base, struct ks2_serdes *serdes)
{
	writel(serdes_cfg_pll_enable[serdes->intf],
	       base + SERDES_PLL_CTL_REG);
}

static void ks2_serdes_lane_reset(u32 base, u32 reset, u32 lane)
{
	if (reset)
		ks2_serdes_rmw(base + SERDES_LANE_REG_028(lane),
			       0x1, SERDES_LANE_RESET);
	else
		ks2_serdes_rmw(base + SERDES_LANE_REG_028(lane),
			       0x0, SERDES_LANE_RESET);
}

static void ks2_serdes_lane_enable(u32 base,
				   struct ks2_serdes *serdes, u32 lane)
{
	/* Bring lane out of reset */
	ks2_serdes_lane_reset(base, 0, lane);

	writel(SERDES_LANE_EN_VAL(serdes_cfg_lane_enable, serdes->intf,
				  serdes->rate_mode),
	       base + SERDES_LANE_CTL_STATUS_REG(lane));

	/* Set NES bit if Loopback Enabled */
	if (serdes->loopback)
		ks2_serdes_rmw(base + SERDES_LANE_REG_000(lane),
			       0x1, SERDES_LANE_LOOPBACK);
}

int ks2_serdes_init(u32 base, struct ks2_serdes *serdes, u32 num_lanes)
{
	int i;
	int ret = 0;

	for (i = 0; i < ARRAY_SIZE(cfgs); i++)
		if (serdes->clk == cfgs[i].clk && serdes->rate == cfgs[i].rate)
			break;

	if (i >= ARRAY_SIZE(cfgs)) {
		puts("Cannot find keystone SerDes configuration");
		return -EINVAL;
	}

	ks2_serdes_init_cfg(base, &cfgs[i], num_lanes);

	ks2_serdes_cmu_comlane_enable(base, serdes);
	for (i = 0; i < num_lanes; i++)
		ks2_serdes_lane_enable(base, serdes, i);

	ks2_serdes_pll_enable(base, serdes);

	return ret;
}
