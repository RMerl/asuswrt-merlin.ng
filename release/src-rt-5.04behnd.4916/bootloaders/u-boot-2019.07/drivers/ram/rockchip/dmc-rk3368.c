// SPDX-License-Identifier: GPL-2.0
/*
 * (C) Copyright 2017 Theobroma Systems Design und Consulting GmbH
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <dt-bindings/memory/rk3368-dmc.h>
#include <dt-structs.h>
#include <ram.h>
#include <regmap.h>
#include <syscon.h>
#include <asm/io.h>
#include <asm/arch-rockchip/clock.h>
#include <asm/arch-rockchip/cru_rk3368.h>
#include <asm/arch-rockchip/grf_rk3368.h>
#include <asm/arch-rockchip/ddr_rk3368.h>
#include <asm/arch-rockchip/sdram.h>
#include <asm/arch-rockchip/sdram_common.h>

struct dram_info {
	struct ram_info info;
	struct clk ddr_clk;
	struct rk3368_cru *cru;
	struct rk3368_grf *grf;
	struct rk3368_ddr_pctl *pctl;
	struct rk3368_ddrphy *phy;
	struct rk3368_pmu_grf *pmugrf;
	struct rk3368_msch *msch;
};

struct rk3368_sdram_params {
#if CONFIG_IS_ENABLED(OF_PLATDATA)
	struct dtd_rockchip_rk3368_dmc of_plat;
#endif
	struct rk3288_sdram_pctl_timing pctl_timing;
	u32 trefi_mem_ddr3;
	struct rk3288_sdram_channel chan;
	struct regmap *map;
	u32 ddr_freq;
	u32 memory_schedule;
	u32 ddr_speed_bin;
	u32 tfaw_mult;
};

/* PTCL bits */
enum {
	/* PCTL_DFISTCFG0 */
	DFI_INIT_START = BIT(0),
	DFI_DATA_BYTE_DISABLE_EN = BIT(2),

	/* PCTL_DFISTCFG1 */
	DFI_DRAM_CLK_SR_EN = BIT(0),
	DFI_DRAM_CLK_DPD_EN = BIT(1),
	ODT_LEN_BL8_W_SHIFT = 16,

	/* PCTL_DFISTCFG2 */
	DFI_PARITY_INTR_EN = BIT(0),
	DFI_PARITY_EN = BIT(1),

	/* PCTL_DFILPCFG0 */
	TLP_RESP_TIME_SHIFT = 16,
	LP_SR_EN = BIT(8),
	LP_PD_EN = BIT(0),

	/* PCTL_DFIODTCFG */
	RANK0_ODT_WRITE_SEL = BIT(3),
	RANK1_ODT_WRITE_SEL = BIT(11),

	/* PCTL_SCFG */
	HW_LOW_POWER_EN = BIT(0),

	/* PCTL_MCMD */
	START_CMD = BIT(31),
	MCMD_RANK0 = BIT(20),
	MCMD_RANK1 = BIT(21),
	DESELECT_CMD = 0,
	PREA_CMD,
	REF_CMD,
	MRS_CMD,
	ZQCS_CMD,
	ZQCL_CMD,
	RSTL_CMD,
	MRR_CMD	= 8,
	DPDE_CMD,

	/* PCTL_POWCTL */
	POWER_UP_START = BIT(0),

	/* PCTL_POWSTAT */
	POWER_UP_DONE = BIT(0),

	/* PCTL_SCTL */
	INIT_STATE = 0,
	CFG_STATE,
	GO_STATE,
	SLEEP_STATE,
	WAKEUP_STATE,

	/* PCTL_STAT */
	LP_TRIG_SHIFT = 4,
	LP_TRIG_MASK = 7,
	PCTL_STAT_MSK = 7,
	INIT_MEM = 0,
	CONFIG,
	CONFIG_REQ,
	ACCESS,
	ACCESS_REQ,
	LOW_POWER,
	LOW_POWER_ENTRY_REQ,
	LOW_POWER_EXIT_REQ,

	/* PCTL_MCFG */
	DDR2_DDR3_BL_8 = BIT(0),
	DDR3_EN = BIT(5),
	TFAW_TRRD_MULT4 = (0 << 18),
	TFAW_TRRD_MULT5 = (1 << 18),
	TFAW_TRRD_MULT6 = (2 << 18),
};

#define DDR3_MR0_WR(n) \
	((n <= 8) ? ((n - 4) << 9) : (((n >> 1) & 0x7) << 9))
#define DDR3_MR0_CL(n) \
	((((n - 4) & 0x7) << 4) | (((n - 4) & 0x8) >> 2))
#define DDR3_MR0_BL8 \
	(0 << 0)
#define DDR3_MR0_DLL_RESET \
	(1 << 8)
#define DDR3_MR1_RTT120OHM \
	((0 << 9) | (1 << 6) | (0 << 2))
#define DDR3_MR2_TWL(n) \
	(((n - 5) & 0x7) << 3)


#ifdef CONFIG_TPL_BUILD

static void ddr_set_noc_spr_err_stall(struct rk3368_grf *grf, bool enable)
{
	if (enable)
		rk_setreg(&grf->ddrc0_con0, NOC_RSP_ERR_STALL);
	else
		rk_clrreg(&grf->ddrc0_con0, NOC_RSP_ERR_STALL);
}

static void ddr_set_ddr3_mode(struct rk3368_grf *grf, bool ddr3_mode)
{
	if (ddr3_mode)
		rk_setreg(&grf->ddrc0_con0, MSCH0_MAINDDR3_DDR3);
	else
		rk_clrreg(&grf->ddrc0_con0, MSCH0_MAINDDR3_DDR3);
}

static void ddrphy_config(struct rk3368_ddrphy *phy,
			  u32 tcl, u32 tal, u32 tcwl)
{
	int i;

	/* Set to DDR3 mode */
	clrsetbits_le32(&phy->reg[1], 0x3, 0x0);

	/* DDRPHY_REGB: CL, AL */
	clrsetbits_le32(&phy->reg[0xb], 0xff, tcl << 4 | tal);
	/* DDRPHY_REGC: CWL */
	clrsetbits_le32(&phy->reg[0xc], 0x0f, tcwl);

	/* Update drive-strength */
	writel(0xcc, &phy->reg[0x11]);
	writel(0xaa, &phy->reg[0x16]);
	/*
	 * Update NRCOMP/PRCOMP for all 4 channels (for details of all
	 * affected registers refer to the documentation of DDRPHY_REG20
	 * and DDRPHY_REG21 in the RK3368 TRM.
	 */
	for (i = 0; i < 4; ++i) {
		writel(0xcc, &phy->reg[0x20 + i * 0x10]);
		writel(0x44, &phy->reg[0x21 + i * 0x10]);
	}

	/* Enable write-leveling calibration bypass */
	setbits_le32(&phy->reg[2], BIT(3));
}

static void copy_to_reg(u32 *dest, const u32 *src, u32 n)
{
	int i;

	for (i = 0; i < n / sizeof(u32); i++)
		writel(*src++, dest++);
}

static void send_command(struct rk3368_ddr_pctl *pctl, u32 rank, u32 cmd)
{
	u32 mcmd = START_CMD | cmd | rank;

	debug("%s: writing %x to MCMD\n", __func__, mcmd);
	writel(mcmd, &pctl->mcmd);
	while (readl(&pctl->mcmd) & START_CMD)
		/* spin */;
}

static void send_mrs(struct rk3368_ddr_pctl *pctl,
			    u32 rank, u32 mr_num, u32 mr_data)
{
	u32 mcmd = START_CMD | MRS_CMD | rank | (mr_num << 17) | (mr_data << 4);

	debug("%s: writing %x to MCMD\n", __func__, mcmd);
	writel(mcmd, &pctl->mcmd);
	while (readl(&pctl->mcmd) & START_CMD)
		/* spin */;
}

static int memory_init(struct rk3368_ddr_pctl *pctl,
		       struct rk3368_sdram_params *params)
{
	u32 mr[4];
	const ulong timeout_ms = 500;
	ulong tmp;

	/*
	 * Power up DRAM by DDR_PCTL_POWCTL[0] register of PCTL and
	 * wait power up DRAM finish with DDR_PCTL_POWSTAT[0] register
	 * of PCTL.
	 */
	writel(POWER_UP_START, &pctl->powctl);

	tmp = get_timer(0);
	do {
		if (get_timer(tmp) > timeout_ms) {
			pr_err("%s: POWER_UP_START did not complete in %ld ms\n",
			      __func__, timeout_ms);
			return -ETIME;
		}
	} while (!(readl(&pctl->powstat) & POWER_UP_DONE));

	/* Configure MR0 through MR3 */
	mr[0] = DDR3_MR0_WR(params->pctl_timing.twr) |
		DDR3_MR0_CL(params->pctl_timing.tcl) |
		DDR3_MR0_DLL_RESET;
	mr[1] = DDR3_MR1_RTT120OHM;
	mr[2] = DDR3_MR2_TWL(params->pctl_timing.tcwl);
	mr[3] = 0;

	/*
	 * Also see RK3368 Technical Reference Manual:
	 *   "16.6.2 Initialization (DDR3 Initialization Sequence)"
	 */
	send_command(pctl, MCMD_RANK0 | MCMD_RANK1, DESELECT_CMD);
	udelay(1);
	send_command(pctl, MCMD_RANK0 | MCMD_RANK1, PREA_CMD);
	send_mrs(pctl, MCMD_RANK0 | MCMD_RANK1, 2, mr[2]);
	send_mrs(pctl, MCMD_RANK0 | MCMD_RANK1, 3, mr[3]);
	send_mrs(pctl, MCMD_RANK0 | MCMD_RANK1, 1, mr[1]);
	send_mrs(pctl, MCMD_RANK0 | MCMD_RANK1, 0, mr[0]);
	send_command(pctl, MCMD_RANK0 | MCMD_RANK1, ZQCL_CMD);

	return 0;
}

static void move_to_config_state(struct rk3368_ddr_pctl *pctl)
{
	/*
	 * Also see RK3368 Technical Reference Manual:
	 *   "16.6.1 State transition of PCTL (Moving to Config State)"
	 */
	u32 state = readl(&pctl->stat) & PCTL_STAT_MSK;

	switch (state) {
	case LOW_POWER:
		writel(WAKEUP_STATE, &pctl->sctl);
		while ((readl(&pctl->stat) & PCTL_STAT_MSK) != ACCESS)
			/* spin */;

		/* fall-through */
	case ACCESS:
	case INIT_MEM:
		writel(CFG_STATE, &pctl->sctl);
		while ((readl(&pctl->stat) & PCTL_STAT_MSK) != CONFIG)
			/* spin */;
		break;

	case CONFIG:
		return;

	default:
		break;
	}
}

static void move_to_access_state(struct rk3368_ddr_pctl *pctl)
{
	/*
	 * Also see RK3368 Technical Reference Manual:
	 *   "16.6.1 State transition of PCTL (Moving to Access State)"
	 */
	u32 state = readl(&pctl->stat) & PCTL_STAT_MSK;

	switch (state) {
	case LOW_POWER:
		if (((readl(&pctl->stat) >> LP_TRIG_SHIFT) &
		     LP_TRIG_MASK) == 1)
			return;

		writel(WAKEUP_STATE, &pctl->sctl);
		while ((readl(&pctl->stat) & PCTL_STAT_MSK) != ACCESS)
			/* spin */;

		/* fall-through */
	case INIT_MEM:
		writel(CFG_STATE, &pctl->sctl);
		while ((readl(&pctl->stat) & PCTL_STAT_MSK) != CONFIG)
			/* spin */;

		/* fall-through */
	case CONFIG:
		writel(GO_STATE, &pctl->sctl);
		while ((readl(&pctl->stat) & PCTL_STAT_MSK) == CONFIG)
			/* spin */;
		break;

	case ACCESS:
		return;

	default:
		break;
	}
}

static void ddrctl_reset(struct rk3368_cru *cru)
{
	const u32 ctl_reset = BIT(3) | BIT(2);
	const u32 phy_reset = BIT(1) | BIT(0);

	/*
	 * The PHY reset should be released before the PCTL reset.
	 *
	 * Note that the following sequence (including the number of
	 * us to delay between releasing the PHY and PCTL reset) has
	 * been adapted per feedback received from Rockchips, so do
	 * not try to optimise.
	 */
	rk_setreg(&cru->softrst_con[10], ctl_reset | phy_reset);
	udelay(1);
	rk_clrreg(&cru->softrst_con[10], phy_reset);
	udelay(5);
	rk_clrreg(&cru->softrst_con[10], ctl_reset);
}

static void ddrphy_reset(struct rk3368_ddrphy *ddrphy)
{
	/*
	 * The analog part of the PHY should be release at least 1000
	 * DRAM cycles before the digital part of the PHY (waiting for
	 * 5us will ensure this for a DRAM clock as low as 200MHz).
	 */
	clrbits_le32(&ddrphy->reg[0], BIT(3) | BIT(2));
	udelay(1);
	setbits_le32(&ddrphy->reg[0], BIT(2));
	udelay(5);
	setbits_le32(&ddrphy->reg[0], BIT(3));
}

static void ddrphy_config_delays(struct rk3368_ddrphy *ddrphy, u32 freq)
{
	u32 dqs_dll_delay;

	setbits_le32(&ddrphy->reg[0x13], BIT(4));
	clrbits_le32(&ddrphy->reg[0x14], BIT(3));

	setbits_le32(&ddrphy->reg[0x26], BIT(4));
	clrbits_le32(&ddrphy->reg[0x27], BIT(3));

	setbits_le32(&ddrphy->reg[0x36], BIT(4));
	clrbits_le32(&ddrphy->reg[0x37], BIT(3));

	setbits_le32(&ddrphy->reg[0x46], BIT(4));
	clrbits_le32(&ddrphy->reg[0x47], BIT(3));

	setbits_le32(&ddrphy->reg[0x56], BIT(4));
	clrbits_le32(&ddrphy->reg[0x57], BIT(3));

	if (freq <= 400000000)
		setbits_le32(&ddrphy->reg[0xa4], 0x1f);
	else
		clrbits_le32(&ddrphy->reg[0xa4], 0x1f);

	if (freq < 681000000)
		dqs_dll_delay = 3; /* 67.5 degree delay */
	else
		dqs_dll_delay = 2; /* 45 degree delay */

	writel(dqs_dll_delay, &ddrphy->reg[0x28]);
	writel(dqs_dll_delay, &ddrphy->reg[0x38]);
	writel(dqs_dll_delay, &ddrphy->reg[0x48]);
	writel(dqs_dll_delay, &ddrphy->reg[0x58]);
}

static int dfi_cfg(struct rk3368_ddr_pctl *pctl)
{
	const ulong timeout_ms = 200;
	ulong tmp;

	writel(DFI_DATA_BYTE_DISABLE_EN, &pctl->dfistcfg0);

	writel(DFI_DRAM_CLK_SR_EN | DFI_DRAM_CLK_DPD_EN,
	       &pctl->dfistcfg1);
	writel(DFI_PARITY_INTR_EN | DFI_PARITY_EN, &pctl->dfistcfg2);
	writel(7 << TLP_RESP_TIME_SHIFT | LP_SR_EN | LP_PD_EN,
	       &pctl->dfilpcfg0);

	writel(1, &pctl->dfitphyupdtype0);

	writel(0x1f, &pctl->dfitphyrdlat);
	writel(0, &pctl->dfitphywrdata);
	writel(0, &pctl->dfiupdcfg);  /* phyupd and ctrlupd disabled */

	setbits_le32(&pctl->dfistcfg0, DFI_INIT_START);

	tmp = get_timer(0);
	do {
		if (get_timer(tmp) > timeout_ms) {
			pr_err("%s: DFI init did not complete within %ld ms\n",
			      __func__, timeout_ms);
			return -ETIME;
		}
	} while ((readl(&pctl->dfiststat0) & 1) == 0);

	return 0;
}

static inline u32 ps_to_tCK(const u32 ps, const ulong freq)
{
	const ulong MHz = 1000000;
	return DIV_ROUND_UP(ps * freq, 1000000 * MHz);
}

static inline u32 ns_to_tCK(const u32 ns, const ulong freq)
{
	return ps_to_tCK(ns * 1000, freq);
}

static inline u32 tCK_to_ps(const ulong tCK, const ulong freq)
{
	const ulong MHz = 1000000;
	return DIV_ROUND_UP(tCK * 1000000 * MHz, freq);
}

static int pctl_calc_timings(struct rk3368_sdram_params *params,
			      ulong freq)
{
	struct rk3288_sdram_pctl_timing *pctl_timing = &params->pctl_timing;
	const ulong MHz = 1000000;
	u32 tccd;
	u32 tfaw_as_ps;

	if (params->ddr_speed_bin != DDR3_1600K) {
		pr_err("%s: unimplemented DDR3 speed bin %d\n",
		      __func__, params->ddr_speed_bin);
		return -1;
	}

	/* PCTL is clocked at 1/2 the DRAM clock; err on the side of caution */
	pctl_timing->togcnt1u = DIV_ROUND_UP(freq, 2 * MHz);
	pctl_timing->togcnt100n = DIV_ROUND_UP(freq / 10, 2 * MHz);

	pctl_timing->tinit = 200;                 /* 200 usec                */
	pctl_timing->trsth = 500;                 /* 500 usec                */
	pctl_timing->trefi = 78;                  /* 7.8usec = 78 * 100ns    */
	params->trefi_mem_ddr3 = ns_to_tCK(pctl_timing->trefi * 100, freq);

	if (freq <= (400 * MHz)) {
		pctl_timing->tcl = 6;
		pctl_timing->tcwl = 10;
	} else if (freq <= (533 * MHz)) {
		pctl_timing->tcl = 8;
		pctl_timing->tcwl = 6;
	} else if (freq <= (666 * MHz)) {
		pctl_timing->tcl = 10;
		pctl_timing->tcwl = 7;
	} else {
		pctl_timing->tcl = 11;
		pctl_timing->tcwl = 8;
	}

	pctl_timing->tmrd = 4;                    /* 4 tCK (all speed bins)  */
	pctl_timing->trfc = ns_to_tCK(350, freq); /* tRFC: 350 (max) @ 8GBit */
	pctl_timing->trp = max(4u, ps_to_tCK(13750, freq));
	/*
	 * JESD-79:
	 *   READ to WRITE Command Delay = RL + tCCD / 2 + 2tCK - WL
	 */
	tccd = 4;
	pctl_timing->trtw = pctl_timing->tcl + tccd/2 + 2 - pctl_timing->tcwl;
	pctl_timing->tal = 0;
	pctl_timing->tras = ps_to_tCK(35000, freq);
	pctl_timing->trc = ps_to_tCK(48750, freq);
	pctl_timing->trcd = ps_to_tCK(13750, freq);
	pctl_timing->trrd = max(4u, ps_to_tCK(7500, freq));
	pctl_timing->trtp = max(4u, ps_to_tCK(7500, freq));
	pctl_timing->twr = ps_to_tCK(15000, freq);
	/* The DDR3 mode-register does only support even values for tWR > 8. */
	if (pctl_timing->twr > 8)
		pctl_timing->twr = (pctl_timing->twr + 1) & ~1;
	pctl_timing->twtr = max(4u, ps_to_tCK(7500, freq));
	pctl_timing->texsr = 512;                 /* tEXSR(max) is tDLLLK    */
	pctl_timing->txp = max(3u, ps_to_tCK(6000, freq));
	pctl_timing->txpdll = max(10u, ps_to_tCK(24000, freq));
	pctl_timing->tzqcs = max(64u, ps_to_tCK(80000, freq));
	pctl_timing->tzqcsi = 10000;               /* as used by Rockchip    */
	pctl_timing->tdqs = 1;                     /* fixed for DDR3         */
	pctl_timing->tcksre = max(5u, ps_to_tCK(10000, freq));
	pctl_timing->tcksrx = max(5u, ps_to_tCK(10000, freq));
	pctl_timing->tcke = max(3u, ps_to_tCK(5000, freq));
	pctl_timing->tmod = max(12u, ps_to_tCK(15000, freq));
	pctl_timing->trstl = ns_to_tCK(100, freq);
	pctl_timing->tzqcl = max(256u, ps_to_tCK(320000, freq));   /* tZQoper */
	pctl_timing->tmrr = 0;
	pctl_timing->tckesr = pctl_timing->tcke + 1;  /* JESD-79: tCKE + 1tCK */
	pctl_timing->tdpd = 0;    /* RK3368 TRM: "allowed values for DDR3: 0" */


	/*
	 * The controller can represent tFAW as 4x, 5x or 6x tRRD only.
	 * We want to use the smallest multiplier that satisfies the tFAW
	 * requirements of the given speed-bin.  If necessary, we stretch out
	 * tRRD to allow us to operate on a 6x multiplier for tFAW.
	 */
	tfaw_as_ps = 40000;      /* 40ns: tFAW for DDR3-1600K, 2KB page-size */
	if (tCK_to_ps(pctl_timing->trrd * 6, freq) < tfaw_as_ps) {
		/* If tFAW is > 6 x tRRD, we need to stretch tRRD */
		pctl_timing->trrd = ps_to_tCK(DIV_ROUND_UP(40000, 6), freq);
		params->tfaw_mult = TFAW_TRRD_MULT6;
	} else if (tCK_to_ps(pctl_timing->trrd * 5, freq) < tfaw_as_ps) {
		params->tfaw_mult = TFAW_TRRD_MULT6;
	} else if (tCK_to_ps(pctl_timing->trrd * 4, freq) < tfaw_as_ps) {
		params->tfaw_mult = TFAW_TRRD_MULT5;
	} else {
		params->tfaw_mult = TFAW_TRRD_MULT4;
	}

	return 0;
}

static void pctl_cfg(struct rk3368_ddr_pctl *pctl,
		     struct rk3368_sdram_params *params,
		     struct rk3368_grf *grf)
{
	/* Configure PCTL timing registers */
	params->pctl_timing.trefi |= BIT(31);   /* see PCTL_TREFI */
	copy_to_reg(&pctl->togcnt1u, &params->pctl_timing.togcnt1u,
		    sizeof(params->pctl_timing));
	writel(params->trefi_mem_ddr3, &pctl->trefi_mem_ddr3);

	/* Set up ODT write selector and ODT write length */
	writel((RANK0_ODT_WRITE_SEL | RANK1_ODT_WRITE_SEL), &pctl->dfiodtcfg);
	writel(7 << ODT_LEN_BL8_W_SHIFT, &pctl->dfiodtcfg1);

	/* Set up the CL/CWL-dependent timings of DFI */
	writel((params->pctl_timing.tcl - 1) / 2 - 1, &pctl->dfitrddataen);
	writel((params->pctl_timing.tcwl - 1) / 2 - 1, &pctl->dfitphywrlat);

	/* DDR3 */
	writel(params->tfaw_mult | DDR3_EN | DDR2_DDR3_BL_8, &pctl->mcfg);
	writel(0x001c0004, &grf->ddrc0_con0);

	setbits_le32(&pctl->scfg, HW_LOW_POWER_EN);
}

static int ddrphy_data_training(struct rk3368_ddr_pctl *pctl,
				struct rk3368_ddrphy *ddrphy)
{
	const u32 trefi = readl(&pctl->trefi);
	const ulong timeout_ms = 500;
	ulong tmp;

	/* disable auto-refresh */
	writel(0 | BIT(31), &pctl->trefi);

	clrsetbits_le32(&ddrphy->reg[2], 0x33, 0x20);
	clrsetbits_le32(&ddrphy->reg[2], 0x33, 0x21);

	tmp = get_timer(0);
	do {
		if (get_timer(tmp) > timeout_ms) {
			pr_err("%s: did not complete within %ld ms\n",
			      __func__, timeout_ms);
			return -ETIME;
		}
	} while ((readl(&ddrphy->reg[0xff]) & 0xf) != 0xf);

	send_command(pctl, MCMD_RANK0 | MCMD_RANK1, PREA_CMD);
	clrsetbits_le32(&ddrphy->reg[2], 0x33, 0x20);
	/* resume auto-refresh */
	writel(trefi | BIT(31), &pctl->trefi);

	return 0;
}

static int sdram_col_row_detect(struct udevice *dev)
{
	struct dram_info *priv = dev_get_priv(dev);
	struct rk3368_sdram_params *params = dev_get_platdata(dev);
	struct rk3368_ddr_pctl *pctl = priv->pctl;
	struct rk3368_msch *msch = priv->msch;
	const u32 test_pattern = 0x5aa5f00f;
	int row, col;
	uintptr_t addr;

	move_to_config_state(pctl);
	writel(6, &msch->ddrconf);
	move_to_access_state(pctl);

	/* Detect col */
	for (col = 11; col >= 9; col--) {
		writel(0, CONFIG_SYS_SDRAM_BASE);
		addr = CONFIG_SYS_SDRAM_BASE +
			(1 << (col + params->chan.bw - 1));
		writel(test_pattern, addr);
		if ((readl(addr) == test_pattern) &&
		    (readl(CONFIG_SYS_SDRAM_BASE) == 0))
			break;
	}

	if (col == 8) {
		pr_err("%s: col detect error\n", __func__);
		return -EINVAL;
	}

	move_to_config_state(pctl);
	writel(15, &msch->ddrconf);
	move_to_access_state(pctl);

	/* Detect row*/
	for (row = 16; row >= 12; row--) {
		writel(0, CONFIG_SYS_SDRAM_BASE);
		addr = CONFIG_SYS_SDRAM_BASE + (1 << (row + 15 - 1));
		writel(test_pattern, addr);
		if ((readl(addr) == test_pattern) &&
		    (readl(CONFIG_SYS_SDRAM_BASE) == 0))
			break;
	}

	if (row == 11) {
		pr_err("%s: row detect error\n", __func__);
		return -EINVAL;
	}

	/* Record results */
	debug("%s: col %d, row %d\n", __func__, col, row);
	params->chan.col = col;
	params->chan.cs0_row = row;
	params->chan.cs1_row = row;
	params->chan.row_3_4 = 0;

	return 0;
}

static int msch_niu_config(struct rk3368_msch *msch,
			   struct rk3368_sdram_params *params)
{
	int i;
	const u8 cols =	params->chan.col - ((params->chan.bw == 2) ? 0 : 1);
	const u8 rows = params->chan.cs0_row;

	/*
	 * The DDR address-translation table always assumes a 32bit
	 * bus and the comparison below takes care of adjusting for
	 * a 16bit bus (i.e. one column-address is consumed).
	 */
	const struct {
		u8 rows;
		u8 columns;
		u8 type;
	} ddrconf_table[] = {
		/*
		 * C-B-R-D patterns are first. For these we require an
		 * exact match for the columns and rows (as there's
		 * one entry per possible configuration).
		 */
		[0] =  { .rows = 13, .columns = 10, .type = DMC_MSCH_CBRD },
		[1] =  { .rows = 14, .columns = 10, .type = DMC_MSCH_CBRD },
		[2] =  { .rows = 15, .columns = 10, .type = DMC_MSCH_CBRD },
		[3] =  { .rows = 16, .columns = 10, .type = DMC_MSCH_CBRD },
		[4] =  { .rows = 14, .columns = 11, .type = DMC_MSCH_CBRD },
		[5] =  { .rows = 15, .columns = 11, .type = DMC_MSCH_CBRD },
		[6] =  { .rows = 16, .columns = 11, .type = DMC_MSCH_CBRD },
		[7] =  { .rows = 13, .columns = 9, .type = DMC_MSCH_CBRD },
		[8] =  { .rows = 14, .columns = 9, .type = DMC_MSCH_CBRD },
		[9] =  { .rows = 15, .columns = 9, .type = DMC_MSCH_CBRD },
		[10] = { .rows = 16, .columns = 9, .type = DMC_MSCH_CBRD },
		/*
		 * 11 through 13 are C-R-B-D patterns. These are
		 * matched for an exact number of columns and to
		 * ensure that the hardware uses at least as many rows
		 * as the pattern requires (i.e. we make sure that
		 * there's no gaps up until we hit the device/chip-select;
		 * however, these patterns can accept up to 16 rows,
		 * as the row-address continues right after the CS
		 * switching)
		 */
		[11] = { .rows = 15, .columns = 10, .type = DMC_MSCH_CRBD },
		[12] = { .rows = 14, .columns = 11, .type = DMC_MSCH_CRBD },
		[13] = { .rows = 13, .columns = 10, .type = DMC_MSCH_CRBD },
		/*
		 * 14 and 15 are catch-all variants using a C-B-D-R
		 * scheme (i.e. alternating the chip-select every time
		 * C-B overflows) and stuffing the remaining C-bits
		 * into the top. Matching needs to make sure that the
		 * number of columns is either an exact match (i.e. we
		 * can use less the the maximum number of rows) -or-
		 * that the columns exceed what is given in this table
		 * and the rows are an exact match (in which case the
		 * remaining C-bits will be stuffed onto the top after
		 * the device/chip-select switches).
		 */
		[14] = { .rows = 16, .columns = 10, .type = DMC_MSCH_CBDR },
		[15] = { .rows = 16, .columns = 9, .type = DMC_MSCH_CBDR },
	};

	/*
	 * For C-B-R-D, we need an exact match (i.e. both for the number of
	 * columns and rows), while for C-B-D-R, only the the number of
	 * columns needs to match.
	 */
	for (i = 0; i < ARRAY_SIZE(ddrconf_table); i++) {
		bool match = false;

		/* If this entry if for a different matcher, then skip it */
		if (ddrconf_table[i].type != params->memory_schedule)
			continue;

		/*
		 * Match according to the rules (exact/inexact/at-least)
		 * documented in the ddrconf_table above.
		 */
		switch (params->memory_schedule) {
		case DMC_MSCH_CBRD:
			match = (ddrconf_table[i].columns == cols) &&
				(ddrconf_table[i].rows == rows);
			break;

		case DMC_MSCH_CRBD:
			match = (ddrconf_table[i].columns == cols) &&
				(ddrconf_table[i].rows <= rows);
			break;

		case DMC_MSCH_CBDR:
			match = (ddrconf_table[i].columns == cols) ||
				((ddrconf_table[i].columns <= cols) &&
				 (ddrconf_table[i].rows == rows));
			break;

		default:
			break;
		}

		if (match) {
			debug("%s: setting ddrconf 0x%x\n", __func__, i);
			writel(i, &msch->ddrconf);
			return 0;
		}
	}

	pr_err("%s: ddrconf (NIU config) not found\n", __func__);
	return -EINVAL;
}

static void dram_all_config(struct udevice *dev)
{
	struct dram_info *priv = dev_get_priv(dev);
	struct rk3368_pmu_grf *pmugrf = priv->pmugrf;
	struct rk3368_sdram_params *params = dev_get_platdata(dev);
	const struct rk3288_sdram_channel *info = &params->chan;
	u32 sys_reg = 0;
	const int chan = 0;

	sys_reg |= DDR3 << SYS_REG_DDRTYPE_SHIFT;
	sys_reg |= 0 << SYS_REG_NUM_CH_SHIFT;

	sys_reg |= info->row_3_4 << SYS_REG_ROW_3_4_SHIFT(chan);
	sys_reg |= 1 << SYS_REG_CHINFO_SHIFT(chan);
	sys_reg |= (info->rank - 1) << SYS_REG_RANK_SHIFT(chan);
	sys_reg |= (info->col - 9) << SYS_REG_COL_SHIFT(chan);
	sys_reg |= info->bk == 3 ? 0 : 1 << SYS_REG_BK_SHIFT(chan);
	sys_reg |= (info->cs0_row - 13) << SYS_REG_CS0_ROW_SHIFT(chan);
	sys_reg |= (info->cs1_row - 13) << SYS_REG_CS1_ROW_SHIFT(chan);
	sys_reg |= (2 >> info->bw) << SYS_REG_BW_SHIFT(chan);
	sys_reg |= (2 >> info->dbw) << SYS_REG_DBW_SHIFT(chan);

	writel(sys_reg, &pmugrf->os_reg[2]);
}

static int setup_sdram(struct udevice *dev)
{
	struct dram_info *priv = dev_get_priv(dev);
	struct rk3368_sdram_params *params = dev_get_platdata(dev);

	struct rk3368_ddr_pctl *pctl = priv->pctl;
	struct rk3368_ddrphy *ddrphy = priv->phy;
	struct rk3368_cru *cru = priv->cru;
	struct rk3368_grf *grf = priv->grf;
	struct rk3368_msch *msch = priv->msch;

	int ret;

	/* The input clock (i.e. DPLL) needs to be 2x the DRAM frequency */
	ret = clk_set_rate(&priv->ddr_clk, 2 * params->ddr_freq);
	if (ret < 0) {
		debug("%s: could not set DDR clock: %d\n", __func__, ret);
		return ret;
	}

	/* Update the read-latency for the RK3368 */
	writel(0x32, &msch->readlatency);

	/* Initialise the DDR PCTL and DDR PHY */
	ddrctl_reset(cru);
	ddrphy_reset(ddrphy);
	ddrphy_config_delays(ddrphy, params->ddr_freq);
	dfi_cfg(pctl);
	/* Configure relative system information of grf_ddrc0_con0 register */
	ddr_set_ddr3_mode(grf, true);
	ddr_set_noc_spr_err_stall(grf, true);
	/* Calculate timings */
	pctl_calc_timings(params, params->ddr_freq);
	/* Initialise the device timings in protocol controller */
	pctl_cfg(pctl, params, grf);
	/* Configure AL, CL ... information of PHY registers */
	ddrphy_config(ddrphy,
		      params->pctl_timing.tcl,
		      params->pctl_timing.tal,
		      params->pctl_timing.tcwl);

	/* Initialize DRAM and configure with mode-register values */
	ret = memory_init(pctl, params);
	if (ret)
		goto error;

	move_to_config_state(pctl);
	/* Perform data-training */
	ddrphy_data_training(pctl, ddrphy);
	move_to_access_state(pctl);

	/* TODO(prt): could detect rank in training... */
#ifdef CONFIG_TARGET_EVB_PX5
	params->chan.rank = 1;
#else
	params->chan.rank = 2;
#endif
	/* TODO(prt): bus width is not auto-detected (yet)... */
	params->chan.bw = 2;  /* 32bit wide bus */
	params->chan.dbw = params->chan.dbw;  /* 32bit wide bus */

	/* DDR3 is always 8 bank */
	params->chan.bk = 3;
	/* Detect col and row number */
	ret = sdram_col_row_detect(dev);
	if (ret)
		goto error;

	/* Configure NIU DDR configuration */
	ret = msch_niu_config(msch, params);
	if (ret)
		goto error;

	/* set up OS_REG to communicate w/ next stage and OS */
	dram_all_config(dev);

	return 0;

error:
	printf("DRAM init failed!\n");
	hang();
}
#endif

static int rk3368_dmc_ofdata_to_platdata(struct udevice *dev)
{
	int ret = 0;

#if !CONFIG_IS_ENABLED(OF_PLATDATA)
	struct rk3368_sdram_params *plat = dev_get_platdata(dev);

	ret = regmap_init_mem(dev_ofnode(dev), &plat->map);
	if (ret)
		return ret;
#endif

	return ret;
}

#if CONFIG_IS_ENABLED(OF_PLATDATA)
static int conv_of_platdata(struct udevice *dev)
{
	struct rk3368_sdram_params *plat = dev_get_platdata(dev);
	struct dtd_rockchip_rk3368_dmc *of_plat = &plat->of_plat;

	plat->ddr_freq = of_plat->rockchip_ddr_frequency;
	plat->ddr_speed_bin = of_plat->rockchip_ddr_speed_bin;
	plat->memory_schedule = of_plat->rockchip_memory_schedule;

	return 0;
}
#endif

static int rk3368_dmc_probe(struct udevice *dev)
{
#ifdef CONFIG_TPL_BUILD
	struct rk3368_sdram_params *plat = dev_get_platdata(dev);
	struct rk3368_ddr_pctl *pctl;
	struct rk3368_ddrphy *ddrphy;
	struct rk3368_cru *cru;
	struct rk3368_grf *grf;
	struct rk3368_msch *msch;
	int ret;
	struct udevice *dev_clk;
#endif
	struct dram_info *priv = dev_get_priv(dev);

#if CONFIG_IS_ENABLED(OF_PLATDATA)
	ret = conv_of_platdata(dev);
	if (ret)
		return ret;
#endif

	priv->pmugrf = syscon_get_first_range(ROCKCHIP_SYSCON_PMUGRF);
	debug("%s: pmugrf=%p\n", __func__, priv->pmugrf);

#ifdef CONFIG_TPL_BUILD
	pctl = (struct rk3368_ddr_pctl *)plat->of_plat.reg[0];
	ddrphy = (struct rk3368_ddrphy *)plat->of_plat.reg[2];
	msch = syscon_get_first_range(ROCKCHIP_SYSCON_MSCH);
	grf = syscon_get_first_range(ROCKCHIP_SYSCON_GRF);

	priv->pctl = pctl;
	priv->phy = ddrphy;
	priv->msch = msch;
	priv->grf = grf;

	ret = rockchip_get_clk(&dev_clk);
	if (ret)
		return ret;
	priv->ddr_clk.id = CLK_DDR;
	ret = clk_request(dev_clk, &priv->ddr_clk);
	if (ret)
		return ret;

	cru = rockchip_get_cru();
	priv->cru = cru;
	if (IS_ERR(priv->cru))
		return PTR_ERR(priv->cru);

	ret = setup_sdram(dev);
	if (ret)
		return ret;
#endif

	priv->info.base = 0;
	priv->info.size =
		rockchip_sdram_size((phys_addr_t)&priv->pmugrf->os_reg[2]);

	/*
	* we use the 0x00000000~0xfdffffff space since 0xff000000~0xffffffff
	* is SoC register space (i.e. reserved), and 0xfe000000~0xfeffffff is
	* inaccessible for some IP controller.
	*/
	priv->info.size = min(priv->info.size, (size_t)0xfe000000);

	return 0;
}

static int rk3368_dmc_get_info(struct udevice *dev, struct ram_info *info)
{
	struct dram_info *priv = dev_get_priv(dev);

	*info = priv->info;
	return 0;
}

static struct ram_ops rk3368_dmc_ops = {
	.get_info = rk3368_dmc_get_info,
};


static const struct udevice_id rk3368_dmc_ids[] = {
	{ .compatible = "rockchip,rk3368-dmc" },
	{ }
};

U_BOOT_DRIVER(dmc_rk3368) = {
	.name = "rockchip_rk3368_dmc",
	.id = UCLASS_RAM,
	.of_match = rk3368_dmc_ids,
	.ops = &rk3368_dmc_ops,
	.probe = rk3368_dmc_probe,
	.priv_auto_alloc_size = sizeof(struct dram_info),
	.ofdata_to_platdata = rk3368_dmc_ofdata_to_platdata,
	.probe = rk3368_dmc_probe,
	.priv_auto_alloc_size = sizeof(struct dram_info),
	.platdata_auto_alloc_size = sizeof(struct rk3368_sdram_params),
};
