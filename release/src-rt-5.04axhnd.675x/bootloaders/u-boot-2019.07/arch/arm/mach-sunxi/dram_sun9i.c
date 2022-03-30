// SPDX-License-Identifier: GPL-2.0+
/*
 * sun9i dram controller initialisation
 *
 * (C) Copyright 2007-2015
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Jerry Wang <wangflord@allwinnertech.com>
 *
 * (C) Copyright 2016 Theobroma Systems Design und Consulting GmbH
 *                    Philipp Tomsich <philipp.tomsich@theobroma-systems.com>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <ram.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/dram.h>
#include <asm/arch/sys_proto.h>

#define DRAM_CLK (CONFIG_DRAM_CLK * 1000000)

/*
 * The following amounts to an extensive rewrite of the code received from
 * Allwinner as part of the open-source bootloader release (refer to
 * https://github.com/allwinner-zh/bootloader.git) and augments the upstream
 * sources (which act as the primary reference point for the inner workings
 * of the 'underdocumented' DRAM controller in the A80) using the following
 * documentation for other memory controllers based on the (Synopsys)
 * Designware IP (DDR memory protocol controller and DDR PHY)
 *   * TI Keystone II Architecture: DDR3 Memory Controller, User's Guide
 *     Document 'SPRUHN7C', Oct 2013 (revised March 2015)
 *   * Xilinx Zynq UltraScale+ MPSoC Register Reference
 *     document ug1087 (v1.0)
 * Note that the Zynq-documentation provides a very close match for the DDR
 * memory protocol controller (and provides a very good guide to the rounding
 * rules for various timings), whereas the TI Keystone II document should be
 * referred to for DDR PHY specifics only.
 *
 * The DRAM controller in the A80 runs at half the frequency of the DDR PHY
 * (i.e. the rules for MEMC_FREQ_RATIO=2 from the Zynq-documentation apply).
 *
 * Known limitations
 * =================
 * In the current state, the following features are not fully supported and
 * a number of simplifying assumptions have been made:
 *   1) Only DDR3 support is implemented, as our test platform (the A80-Q7
 *      module) is designed to accomodate DDR3/DDR3L.
 *   2) Only 2T-mode has been implemented and tested.
 *   3) The controller supports two different clocking strategies (PLL6 can
 *      either be 2*CK or CK/2)... we only support the 2*CK clock at this
 *      time and haven't verified whether the alternative clocking strategy
 *      works.  If you are interested in porting this over/testing this,
 *      please refer to cases where bit 0 of 'dram_tpr8' is tested in the
 *      original code from Allwinner.
 *   4) Support for 2 ranks per controller is not implemented (as we don't
 *      the hardware to test it).
 *
 * Future directions
 * =================
 * The driver should be driven from a device-tree based configuration that
 * can dynamically provide the necessary timing parameters (i.e. target
 * frequency and speed-bin information)---the data structures used in the
 * calculation of the timing parameters are already designed to capture
 * similar information as the device tree would provide.
 *
 * To enable a device-tree based configuration of the sun9i platform, we
 * will need to enable CONFIG_TPL and bootstrap in 3 stages: initially
 * into SRAM A1 (40KB) and next into SRAM A2 (160KB)---which would be the
 * stage to initialise the platform via the device-tree---before having
 * the full U-Boot run from DDR.
 */

/*
 * A number of DDR3 timings are given as "the greater of a fixed number of
 * clock cycles (CK) or nanoseconds.  We express these using a structure
 * that holds a cycle count and a duration in picoseconds (so we can model
 * sub-ns timings, such as 7.5ns without losing precision or resorting to
 * rounding up early.
 */
struct dram_sun9i_timing {
	u32 ck;
	u32 ps;
};

/* */
struct dram_sun9i_cl_cwl_timing {
	u32 CL;
	u32 CWL;
	u32 tCKmin;  /* in ps */
	u32 tCKmax;  /* in ps */
};

struct dram_sun9i_para {
	u32 dram_type;

	u8 bus_width;
	u8 chan;
	u8 rank;
	u8 rows;
	u16 page_size;

	/* Timing information for each speed-bin */
	struct dram_sun9i_cl_cwl_timing *cl_cwl_table;
	u32 cl_cwl_numentries;

	/*
	 * For the timings, we try to keep the order and grouping used in
	 * JEDEC Standard No. 79-3F
	 */

	/* timings */
	u32 tREFI; /* in ns */
	u32 tRFC;  /* in ns */

	u32 tRAS;  /* in ps */

	/* command and address timing */
	u32 tDLLK; /* in nCK */
	struct dram_sun9i_timing tRTP;
	struct dram_sun9i_timing tWTR;
	u32 tWR;   /* in nCK */
	u32 tMRD;  /* in nCK */
	struct dram_sun9i_timing tMOD;
	u32 tRCD;  /* in ps */
	u32 tRP;   /* in ps */
	u32 tRC;   /* in ps */
	u32 tCCD;  /* in nCK */
	struct dram_sun9i_timing tRRD;
	u32 tFAW;  /* in ps */

	/* calibration timing */
	/* struct dram_sun9i_timing tZQinit; */
	struct dram_sun9i_timing tZQoper;
	struct dram_sun9i_timing tZQCS;

	/* reset timing */
	/* struct dram_sun9i_timing tXPR; */

	/* self-refresh timings */
	struct dram_sun9i_timing tXS;
	u32 tXSDLL; /* in nCK */
	/* struct dram_sun9i_timing tCKESR; */
	struct dram_sun9i_timing tCKSRE;
	struct dram_sun9i_timing tCKSRX;

	/* power-down timings */
	struct dram_sun9i_timing tXP;
	struct dram_sun9i_timing tXPDLL;
	struct dram_sun9i_timing tCKE;

	/* write leveling timings */
	u32 tWLMRD;    /* min, in nCK */
	/* u32 tWLDQSEN;  min, in nCK */
	u32 tWLO;      /* max, in ns */
	/* u32 tWLOE;     max, in ns */

	/* u32 tCKDPX;    in nCK */
	/* u32 tCKCSX;    in nCK */
};

static void mctl_sys_init(void);

#define SCHED_RDWR_IDLE_GAP(n)            ((n & 0xff) << 24)
#define SCHED_GO2CRITICAL_HYSTERESIS(n)   ((n & 0xff) << 16)
#define SCHED_LPR_NUM_ENTRIES(n)          ((n & 0xff) <<  8)
#define SCHED_PAGECLOSE                   (1 << 2)
#define SCHED_PREFER_WRITE                (1 << 1)
#define SCHED_FORCE_LOW_PRI_N             (1 << 0)

#define SCHED_CONFIG		(SCHED_RDWR_IDLE_GAP(0xf) | \
				 SCHED_GO2CRITICAL_HYSTERESIS(0x80) | \
				 SCHED_LPR_NUM_ENTRIES(0x20) | \
				 SCHED_FORCE_LOW_PRI_N)
#define PERFHPR0_CONFIG                   0x0000001f
#define PERFHPR1_CONFIG                   0x1f00001f
#define PERFLPR0_CONFIG                   0x000000ff
#define PERFLPR1_CONFIG                   0x0f0000ff
#define PERFWR0_CONFIG                    0x000000ff
#define PERFWR1_CONFIG                    0x0f0001ff

static void mctl_ctl_sched_init(unsigned long  base)
{
	struct sunxi_mctl_ctl_reg *mctl_ctl =
		(struct sunxi_mctl_ctl_reg *)base;

	/* Needs to be done before the global clk enable... */
	writel(SCHED_CONFIG, &mctl_ctl->sched);
	writel(PERFHPR0_CONFIG, &mctl_ctl->perfhpr0);
	writel(PERFHPR1_CONFIG, &mctl_ctl->perfhpr1);
	writel(PERFLPR0_CONFIG, &mctl_ctl->perflpr0);
	writel(PERFLPR1_CONFIG, &mctl_ctl->perflpr1);
	writel(PERFWR0_CONFIG, &mctl_ctl->perfwr0);
	writel(PERFWR1_CONFIG, &mctl_ctl->perfwr1);
}

static void mctl_sys_init(void)
{
	struct sunxi_ccm_reg * const ccm =
		(struct sunxi_ccm_reg *)SUNXI_CCM_BASE;
	struct sunxi_mctl_com_reg * const mctl_com =
		(struct sunxi_mctl_com_reg *)SUNXI_DRAM_COM_BASE;

	debug("Setting PLL6 to %d\n", DRAM_CLK * 2);
	clock_set_pll6(DRAM_CLK * 2);

	/* Original dram init code which may come in handy later
	********************************************************
	clock_set_pll6(use_2channelPLL ? (DRAM_CLK * 2) :
					 (DRAM_CLK / 2), false);

	if ((para->dram_clk <= 400)|((para->dram_tpr8 & 0x1)==0)) {
		 * PLL6 should be 2*CK *
		 * ccm_setup_pll6_ddr_clk(PLL6_DDR_CLK); *
		ccm_setup_pll6_ddr_clk((1000000 * (para->dram_clk) * 2), 0);
	} else {
		 * PLL6 should be CK/2 *
		ccm_setup_pll6_ddr_clk((1000000 * (para->dram_clk) / 2), 1);
	}

	if (para->dram_tpr13 & (0xf<<18)) {
		 *
		 * bit21:bit18=0001:pll swing 0.4
		 * bit21:bit18=0010:pll swing 0.3
		 * bit21:bit18=0100:pll swing 0.2
		 * bit21:bit18=1000:pll swing 0.1
		 *
		dram_dbg("DRAM fre extend open !\n");
		reg_val=mctl_read_w(CCM_PLL6_DDR_REG);
		reg_val&=(0x1<<16);
		reg_val=reg_val>>16;

		if(para->dram_tpr13 & (0x1<<18))
		{
			mctl_write_w(CCM_PLL_BASE + 0x114,
				(0x3333U|(0x3<<17)|(reg_val<<19)|(0x120U<<20)|
				(0x2U<<29)|(0x1U<<31)));
		}
		else if(para->dram_tpr13 & (0x1<<19))
		{
			mctl_write_w(CCM_PLL_BASE + 0x114,
				(0x6666U|(0x3U<<17)|(reg_val<<19)|(0xD8U<<20)|
				(0x2U<<29)|(0x1U<<31)));
		}
		else if(para->dram_tpr13 & (0x1<<20))
		{
			mctl_write_w(CCM_PLL_BASE + 0x114,
				(0x9999U|(0x3U<<17)|(reg_val<<19)|(0x90U<<20)|
				(0x2U<<29)|(0x1U<<31)));
		}
		else if(para->dram_tpr13 & (0x1<<21))
		{
			mctl_write_w(CCM_PLL_BASE + 0x114,
				(0xccccU|(0x3U<<17)|(reg_val<<19)|(0x48U<<20)|
				(0x2U<<29)|(0x1U<<31)));
		}

		//frequency extend open
		reg_val = mctl_read_w(CCM_PLL6_DDR_REG);
		reg_val |= ((0x1<<24)|(0x1<<30));
		mctl_write_w(CCM_PLL6_DDR_REG, reg_val);


		while(mctl_read_w(CCM_PLL6_DDR_REG) & (0x1<<30));
	}

	aw_delay(0x20000);	//make some delay
	********************************************************
	*/

	/* assert mctl reset */
	clrbits_le32(&ccm->ahb_reset0_cfg, 1 << AHB_RESET_OFFSET_MCTL);
	/* stop mctl clock */
	clrbits_le32(&ccm->ahb_gate0, 1 << AHB_GATE_OFFSET_MCTL);

	sdelay(2000);

	/* deassert mctl reset */
	setbits_le32(&ccm->ahb_reset0_cfg, 1 << AHB_RESET_OFFSET_MCTL);
	/* enable mctl clock */
	setbits_le32(&ccm->ahb_gate0, 1 << AHB_GATE_OFFSET_MCTL);

	/* set up the transactions scheduling before enabling the global clk */
	mctl_ctl_sched_init(SUNXI_DRAM_CTL0_BASE);
	mctl_ctl_sched_init(SUNXI_DRAM_CTL1_BASE);
	sdelay(1000);

	debug("2\n");

	/* (3 << 12): PLL_DDR */
	writel((3 << 12) | (1 << 16), &ccm->dram_clk_cfg);
	do {
		debug("Waiting for DRAM_CLK_CFG\n");
		sdelay(10000);
	} while (readl(&ccm->dram_clk_cfg) & (1 << 16));
	setbits_le32(&ccm->dram_clk_cfg, (1 << 31));

	/* TODO: we only support the common case ... i.e. 2*CK */
	setbits_le32(&mctl_com->ccr, (1 << 14) | (1 << 30));
	writel(2, &mctl_com->rmcr); /* controller clock is PLL6/4 */

	sdelay(2000);

	/* Original dram init code which may come in handy later
	********************************************************
	if ((para->dram_clk <= 400) | ((para->dram_tpr8 & 0x1) == 0)) {
		 * PLL6 should be 2*CK *
		 * gating 2 channel pll *
		reg_val = mctl_read_w(MC_CCR);
		reg_val |= ((0x1 << 14) | (0x1U << 30));
		mctl_write_w(MC_CCR, reg_val);
		mctl_write_w(MC_RMCR, 0x2); * controller clock use pll6/4 *
	} else {
		 * enable 2 channel pll *
		reg_val = mctl_read_w(MC_CCR);
		reg_val &= ~((0x1 << 14) | (0x1U << 30));
		mctl_write_w(MC_CCR, reg_val);
		mctl_write_w(MC_RMCR, 0x0); * controller clock use pll6 *
	}

	reg_val = mctl_read_w(MC_CCR);
	reg_val &= ~((0x1<<15)|(0x1U<<31));
	mctl_write_w(MC_CCR, reg_val);
	aw_delay(20);
	//aw_delay(0x10);
	********************************************************
	*/

	clrbits_le32(&mctl_com->ccr, MCTL_CCR_CH0_CLK_EN | MCTL_CCR_CH1_CLK_EN);
	sdelay(1000);

	setbits_le32(&mctl_com->ccr, MCTL_CCR_CH0_CLK_EN);
	/* TODO if (para->chan == 2) */
	setbits_le32(&mctl_com->ccr, MCTL_CCR_CH1_CLK_EN);
}

static void mctl_com_init(struct dram_sun9i_para *para)
{
	struct sunxi_mctl_com_reg * const mctl_com =
		(struct sunxi_mctl_com_reg *)SUNXI_DRAM_COM_BASE;

	/* TODO: hard-wired for DDR3 now */
	writel(((para->chan == 2) ? MCTL_CR_CHANNEL_DUAL :
				    MCTL_CR_CHANNEL_SINGLE)
	       | MCTL_CR_DRAMTYPE_DDR3 | MCTL_CR_BANK(1)
	       | MCTL_CR_ROW(para->rows)
	       | ((para->bus_width == 32) ? MCTL_CR_BUSW32 : MCTL_CR_BUSW16)
	       | MCTL_CR_PAGE_SIZE(para->page_size) | MCTL_CR_RANK(para->rank),
	       &mctl_com->cr);

	debug("CR: %d\n", readl(&mctl_com->cr));
}

static u32 mctl_channel_init(u32 ch_index, struct dram_sun9i_para *para)
{
	struct sunxi_mctl_ctl_reg *mctl_ctl;
	struct sunxi_mctl_phy_reg *mctl_phy;

	u32 CL = 0;
	u32 CWL = 0;
	u16 mr[4] = { 0, };

#define PS2CYCLES_FLOOR(n)    ((n * CONFIG_DRAM_CLK) / 1000000)
#define PS2CYCLES_ROUNDUP(n)  ((n * CONFIG_DRAM_CLK + 999999) / 1000000)
#define NS2CYCLES_FLOOR(n)    ((n * CONFIG_DRAM_CLK) / 1000)
#define NS2CYCLES_ROUNDUP(n)  ((n * CONFIG_DRAM_CLK + 999) / 1000)
#define MAX(a, b)             ((a) > (b) ? (a) : (b))

	/*
	 * Convert the values to cycle counts (nCK) from what is provided
	 * by the definition of each speed bin.
	 */
	/* const u32 tREFI = NS2CYCLES_FLOOR(para->tREFI); */
	const u32 tREFI = NS2CYCLES_FLOOR(para->tREFI);
	const u32 tRFC  = NS2CYCLES_ROUNDUP(para->tRFC);
	const u32 tRCD  = PS2CYCLES_ROUNDUP(para->tRCD);
	const u32 tRP   = PS2CYCLES_ROUNDUP(para->tRP);
	const u32 tRC   = PS2CYCLES_ROUNDUP(para->tRC);
	const u32 tRAS  = PS2CYCLES_ROUNDUP(para->tRAS);

	/* command and address timing */
	const u32 tDLLK = para->tDLLK;
	const u32 tRTP  = MAX(para->tRTP.ck, PS2CYCLES_ROUNDUP(para->tRTP.ps));
	const u32 tWTR  = MAX(para->tWTR.ck, PS2CYCLES_ROUNDUP(para->tWTR.ps));
	const u32 tWR   = NS2CYCLES_FLOOR(para->tWR);
	const u32 tMRD  = para->tMRD;
	const u32 tMOD  = MAX(para->tMOD.ck, PS2CYCLES_ROUNDUP(para->tMOD.ps));
	const u32 tCCD  = para->tCCD;
	const u32 tRRD  = MAX(para->tRRD.ck, PS2CYCLES_ROUNDUP(para->tRRD.ps));
	const u32 tFAW  = PS2CYCLES_ROUNDUP(para->tFAW);

	/* calibration timings */
	/* const u32 tZQinit = MAX(para->tZQinit.ck,
				PS2CYCLES_ROUNDUP(para->tZQinit.ps)); */
	const u32 tZQoper = MAX(para->tZQoper.ck,
				PS2CYCLES_ROUNDUP(para->tZQoper.ps));
	const u32 tZQCS   = MAX(para->tZQCS.ck,
				PS2CYCLES_ROUNDUP(para->tZQCS.ps));

	/* reset timing */
	/* const u32 tXPR  = MAX(para->tXPR.ck,
				PS2CYCLES_ROUNDUP(para->tXPR.ps)); */

	/* power-down timings */
	const u32 tXP    = MAX(para->tXP.ck, PS2CYCLES_ROUNDUP(para->tXP.ps));
	const u32 tXPDLL = MAX(para->tXPDLL.ck,
			       PS2CYCLES_ROUNDUP(para->tXPDLL.ps));
	const u32 tCKE   = MAX(para->tCKE.ck, PS2CYCLES_ROUNDUP(para->tCKE.ps));

	/*
	 * self-refresh timings (keep below power-down timings, as tCKESR
	 * needs to be calculated based on the nCK value of tCKE)
	 */
	const u32 tXS    = MAX(para->tXS.ck, PS2CYCLES_ROUNDUP(para->tXS.ps));
	const u32 tXSDLL = para->tXSDLL;
	const u32 tCKSRE = MAX(para->tCKSRE.ck,
			       PS2CYCLES_ROUNDUP(para->tCKSRE.ps));
	const u32 tCKESR = tCKE + 1;
	const u32 tCKSRX = MAX(para->tCKSRX.ck,
			       PS2CYCLES_ROUNDUP(para->tCKSRX.ps));

	/* write leveling timings */
	const u32 tWLMRD = para->tWLMRD;
	/* const u32 tWLDQSEN = para->tWLDQSEN; */
	const u32 tWLO = PS2CYCLES_FLOOR(para->tWLO);
	/* const u32 tWLOE = PS2CYCLES_FLOOR(para->tWLOE); */

	const u32 tRASmax = tREFI * 9;
	int i;

	for (i = 0; i < para->cl_cwl_numentries; ++i) {
		const u32 tCK = 1000000 / CONFIG_DRAM_CLK;

		if ((para->cl_cwl_table[i].tCKmin <= tCK) &&
		    (tCK < para->cl_cwl_table[i].tCKmax)) {
			CL = para->cl_cwl_table[i].CL;
			CWL = para->cl_cwl_table[i].CWL;

			debug("found CL/CWL: CL = %d, CWL = %d\n", CL, CWL);
			break;
		}
	}

	if ((CL == 0) && (CWL == 0)) {
		printf("failed to find valid CL/CWL for operating point %d MHz\n",
		       CONFIG_DRAM_CLK);
		return 0;
	}

	if (ch_index == 0) {
		mctl_ctl = (struct sunxi_mctl_ctl_reg *)SUNXI_DRAM_CTL0_BASE;
		mctl_phy = (struct sunxi_mctl_phy_reg *)SUNXI_DRAM_PHY0_BASE;
	} else {
		mctl_ctl = (struct sunxi_mctl_ctl_reg *)SUNXI_DRAM_CTL1_BASE;
		mctl_phy = (struct sunxi_mctl_phy_reg *)SUNXI_DRAM_PHY1_BASE;
	}

	if (para->dram_type == DRAM_TYPE_DDR3) {
		mr[0] = DDR3_MR0_PPD_FAST_EXIT | DDR3_MR0_WR(tWR) |
			DDR3_MR0_CL(CL);
		mr[1] = DDR3_MR1_RTT120OHM;
		mr[2] = DDR3_MR2_TWL(CWL);
		mr[3] = 0;

		/*
		 * DRAM3 initialisation requires holding CKE LOW for
		 * at least 500us prior to starting the initialisation
		 * sequence and at least 10ns after driving CKE HIGH
		 * before the initialisation sequence may be started).
		 *
		 * Refer to Micron document "TN-41-07: DDR3 Power-Up,
		 * Initialization, and Reset DDR3 Initialization
		 * Routine" for details).
		 */
		writel(MCTL_INIT0_POST_CKE_x1024(1) |
		       MCTL_INIT0_PRE_CKE_x1024(
			    (500 * CONFIG_DRAM_CLK + 1023) / 1024), /* 500us */
		       &mctl_ctl->init[0]);
		writel(MCTL_INIT1_DRAM_RSTN_x1024(1),
		       &mctl_ctl->init[1]);
		/* INIT2 is not used for DDR3 */
		writel(MCTL_INIT3_MR(mr[0]) | MCTL_INIT3_EMR(mr[1]),
		       &mctl_ctl->init[3]);
		writel(MCTL_INIT4_EMR2(mr[2]) | MCTL_INIT4_EMR3(mr[3]),
		       &mctl_ctl->init[4]);
		writel(MCTL_INIT5_DEV_ZQINIT_x32(512 / 32), /* 512 cycles */
		       &mctl_ctl->init[5]);
	} else {
		/* !!! UNTESTED !!! */
		/*
		 * LPDDR2 and/or LPDDR3 require a 200us minimum delay
		 * after driving CKE HIGH in the initialisation sequence.
		 */
		writel(MCTL_INIT0_POST_CKE_x1024(
				(200 * CONFIG_DRAM_CLK + 1023) / 1024),
		       &mctl_ctl->init[0]);
		writel(MCTL_INIT1_DRAM_RSTN_x1024(1),
		       &mctl_ctl->init[1]);
		writel(MCTL_INIT2_IDLE_AFTER_RESET_x32(
				(CONFIG_DRAM_CLK + 31) / 32) /* 1us */
		       | MCTL_INIT2_MIN_STABLE_CLOCK_x1(5),  /* 5 cycles */
		       &mctl_ctl->init[2]);
		writel(MCTL_INIT3_MR(mr[1]) | MCTL_INIT3_EMR(mr[2]),
		       &mctl_ctl->init[3]);
		writel(MCTL_INIT4_EMR2(mr[3]),
		       &mctl_ctl->init[4]);
		writel(MCTL_INIT5_DEV_ZQINIT_x32(
				(CONFIG_DRAM_CLK + 31) / 32) /* 1us */
		       | MCTL_INIT5_MAX_AUTO_INIT_x1024(
				(10 * CONFIG_DRAM_CLK + 1023) / 1024),
		       &mctl_ctl->init[5]);
	}

	/* (DDR3) We always use a burst-length of 8. */
#define MCTL_BL               8
	/* wr2pre: WL + BL/2 + tWR */
#define WR2PRE           (MCTL_BL/2 + CWL + tWTR)
	/* wr2rd = CWL + BL/2 + tWTR */
#define WR2RD            (MCTL_BL/2 + CWL + tWTR)
	/*
	 * rd2wr = RL + BL/2 + 2 - WL (for DDR3)
	 * rd2wr = RL + BL/2 + RU(tDQSCKmax/tCK) + 1 - WL (for LPDDR2/LPDDR3)
	 */
#define RD2WR            (CL + MCTL_BL/2 + 2 - CWL)
#define MCTL_PHY_TRTW        0
#define MCTL_PHY_TRTODT      0

#define MCTL_DIV2(n)         ((n + 1)/2)
#define MCTL_DIV32(n)        (n/32)
#define MCTL_DIV1024(n)      (n/1024)

	writel((MCTL_DIV2(WR2PRE) << 24) | (MCTL_DIV2(tFAW) << 16) |
	       (MCTL_DIV1024(tRASmax) << 8) | (MCTL_DIV2(tRAS) << 0),
	       &mctl_ctl->dramtmg[0]);
	writel((MCTL_DIV2(tXP) << 16) | (MCTL_DIV2(tRTP) << 8) |
	       (MCTL_DIV2(tRC) << 0),
	       &mctl_ctl->dramtmg[1]);
	writel((MCTL_DIV2(CWL) << 24) | (MCTL_DIV2(CL) << 16) |
	       (MCTL_DIV2(RD2WR) << 8) | (MCTL_DIV2(WR2RD) << 0),
	       &mctl_ctl->dramtmg[2]);
	/*
	 * Note: tMRW is located at bit 16 (and up) in DRAMTMG3...
	 * this is only relevant for LPDDR2/LPDDR3
	 */
	writel((MCTL_DIV2(tMRD) << 12) | (MCTL_DIV2(tMOD) << 0),
	       &mctl_ctl->dramtmg[3]);
	writel((MCTL_DIV2(tRCD) << 24) | (MCTL_DIV2(tCCD) << 16) |
	       (MCTL_DIV2(tRRD) << 8) | (MCTL_DIV2(tRP) << 0),
	       &mctl_ctl->dramtmg[4]);
	writel((MCTL_DIV2(tCKSRX) << 24) | (MCTL_DIV2(tCKSRE) << 16) |
	       (MCTL_DIV2(tCKESR) << 8) | (MCTL_DIV2(tCKE) << 0),
	       &mctl_ctl->dramtmg[5]);

	/* These timings are relevant for LPDDR2/LPDDR3 only */
	/* writel((MCTL_TCKDPDE << 24) | (MCTL_TCKDPX << 16) |
	       (MCTL_TCKCSX << 0), &mctl_ctl->dramtmg[6]); */

	/* printf("DRAMTMG7 reset value: 0x%x\n",
		readl(&mctl_ctl->dramtmg[7])); */
	/* DRAMTMG7 reset value: 0x202 */
	/* DRAMTMG7 should contain t_ckpde and t_ckpdx: check reset values!!! */
	/* printf("DRAMTMG8 reset value: 0x%x\n",
		readl(&mctl_ctl->dramtmg[8])); */
	/* DRAMTMG8 reset value: 0x44 */

	writel((MCTL_DIV32(tXSDLL) << 0), &mctl_ctl->dramtmg[8]);

	writel((MCTL_DIV32(tREFI) << 16) | (MCTL_DIV2(tRFC) << 0),
	       &mctl_ctl->rfshtmg);

	if (para->dram_type == DRAM_TYPE_DDR3) {
		writel((2 << 24) | ((MCTL_DIV2(CL) - 2) << 16) |
		       (1 << 8) | ((MCTL_DIV2(CWL) - 2) << 0),
			&mctl_ctl->dfitmg[0]);
	} else {
		/* TODO */
	}

	/* TODO: handle the case of the write latency domain going to 0 ... */

	/*
	 * Disable dfi_init_complete_en (the triggering of the SDRAM
	 * initialisation when the PHY initialisation completes).
	 */
	clrbits_le32(&mctl_ctl->dfimisc, MCTL_DFIMISC_DFI_INIT_COMPLETE_EN);
	/* Disable the automatic generation of DLL calibration requests */
	setbits_le32(&mctl_ctl->dfiupd[0], MCTL_DFIUPD0_DIS_AUTO_CTRLUPD);

	/* A80-Q7: 2T, 1 rank, DDR3, full-32bit-DQ */
	/* TODO: make 2T and BUSWIDTH configurable  */
	writel(MCTL_MSTR_DEVICETYPE(para->dram_type) |
	       MCTL_MSTR_BURSTLENGTH(para->dram_type) |
	       MCTL_MSTR_ACTIVERANKS(para->rank) |
	       MCTL_MSTR_2TMODE | MCTL_MSTR_BUSWIDTH32,
	       &mctl_ctl->mstr);

	if (para->dram_type == DRAM_TYPE_DDR3) {
		writel(MCTL_ZQCTRL0_TZQCL(MCTL_DIV2(tZQoper)) |
		       (MCTL_DIV2(tZQCS)), &mctl_ctl->zqctrl[0]);
		/*
		 * TODO: is the following really necessary as the bottom
		 * half should already be 0x100 and the upper half should
		 * be ignored for a DDR3 device???
		 */
		writel(MCTL_ZQCTRL1_TZQSI_x1024(0x100),
		       &mctl_ctl->zqctrl[1]);
	} else {
		writel(MCTL_ZQCTRL0_TZQCL(0x200) | MCTL_ZQCTRL0_TZQCS(0x40),
		       &mctl_ctl->zqctrl[0]);
		writel(MCTL_ZQCTRL1_TZQRESET(0x28) |
		       MCTL_ZQCTRL1_TZQSI_x1024(0x100),
		       &mctl_ctl->zqctrl[1]);
	}

	/* Assert dfi_init_complete signal */
	setbits_le32(&mctl_ctl->dfimisc, MCTL_DFIMISC_DFI_INIT_COMPLETE_EN);
	/* Disable auto-refresh */
	setbits_le32(&mctl_ctl->rfshctl3, MCTL_RFSHCTL3_DIS_AUTO_REFRESH);

	/* PHY initialisation */

	/* TODO: make 2T and 8-bank mode configurable  */
	writel(MCTL_PHY_DCR_BYTEMASK | MCTL_PHY_DCR_2TMODE |
	       MCTL_PHY_DCR_DDR8BNK | MCTL_PHY_DRAMMODE_DDR3,
	       &mctl_phy->dcr);

	/* For LPDDR2 or LPDDR3, set DQSGX to 0 before training. */
	if (para->dram_type != DRAM_TYPE_DDR3)
		clrbits_le32(&mctl_phy->dsgcr, (3 << 6));

	writel(mr[0], &mctl_phy->mr0);
	writel(mr[1], &mctl_phy->mr1);
	writel(mr[2], &mctl_phy->mr2);
	writel(mr[3], &mctl_phy->mr3);

	/*
	 * The DFI PHY is running at full rate. We thus use the actual
	 * timings in clock cycles here.
	 */
	writel((tRC << 26) | (tRRD << 22) | (tRAS << 16) |
	       (tRCD << 12) | (tRP << 8) | (tWTR << 4) | (tRTP << 0),
		&mctl_phy->dtpr[0]);
	writel((tMRD << 0) | ((tMOD - 12) << 2) | (tFAW << 5) |
	       (tRFC << 11) | (tWLMRD << 20) | (tWLO << 26),
	       &mctl_phy->dtpr[1]);
	writel((tXS << 0) | (MAX(tXP, tXPDLL) << 10) |
	       (tCKE << 15) | (tDLLK << 19) |
	       (MCTL_PHY_TRTODT << 29) | (MCTL_PHY_TRTW << 30) |
	       (((tCCD - 4) & 0x1) << 31),
	       &mctl_phy->dtpr[2]);

	/* tDQSCK and tDQSCKmax are used LPDDR2/LPDDR3 */
	/* writel((tDQSCK << 0) | (tDQSCKMAX << 3), &mctl_phy->dtpr[3]); */

	/*
	 * We use the same values used by Allwinner's Boot0 for the PTR
	 * (PHY timing register) configuration that is tied to the PHY
	 * implementation.
	 */
	writel(0x42C21590, &mctl_phy->ptr[0]);
	writel(0xD05612C0, &mctl_phy->ptr[1]);
	if (para->dram_type == DRAM_TYPE_DDR3) {
		const unsigned int tdinit0 = 500 * CONFIG_DRAM_CLK; /* 500us */
		const unsigned int tdinit1 = (360 * CONFIG_DRAM_CLK + 999) /
			1000; /* 360ns */
		const unsigned int tdinit2 = 200 * CONFIG_DRAM_CLK; /* 200us */
		const unsigned int tdinit3 = CONFIG_DRAM_CLK; /* 1us */

		writel((tdinit1 << 20) | tdinit0, &mctl_phy->ptr[3]);
		writel((tdinit3 << 18) | tdinit2, &mctl_phy->ptr[4]);
	} else {
		/* LPDDR2 or LPDDR3 */
		const unsigned int tdinit0 = (100 * CONFIG_DRAM_CLK + 999) /
			1000; /* 100ns */
		const unsigned int tdinit1 = 200 * CONFIG_DRAM_CLK; /* 200us */
		const unsigned int tdinit2 = 22 * CONFIG_DRAM_CLK; /* 11us */
		const unsigned int tdinit3 = 2 * CONFIG_DRAM_CLK; /* 2us */

		writel((tdinit1 << 20) | tdinit0, &mctl_phy->ptr[3]);
		writel((tdinit3 << 18) | tdinit2, &mctl_phy->ptr[4]);
	}

	/* TEST ME */
	writel(0x00203131, &mctl_phy->acmdlr);

	/* TODO: can we enable this for 2 ranks, even when we don't know yet */
	writel(MCTL_DTCR_DEFAULT | MCTL_DTCR_RANKEN(para->rank),
	       &mctl_phy->dtcr);

	/* TODO: half width */
	debug("DX2GCR0 reset: 0x%x\n", readl(&mctl_phy->dx[2].gcr[0]));
	writel(0x7C000285, &mctl_phy->dx[2].gcr[0]);
	writel(0x7C000285, &mctl_phy->dx[3].gcr[0]);

	clrsetbits_le32(&mctl_phy->zq[0].pr, 0xff,
			(CONFIG_DRAM_ZQ >>  0) & 0xff);  /* CK/CA */
	clrsetbits_le32(&mctl_phy->zq[1].pr, 0xff,
			(CONFIG_DRAM_ZQ >>  8) & 0xff);  /* DX0/DX1 */
	clrsetbits_le32(&mctl_phy->zq[2].pr, 0xff,
			(CONFIG_DRAM_ZQ >> 16) & 0xff);  /* DX2/DX3 */

	/* TODO: make configurable & implement non-ODT path */
	if (1) {
		int lane;
		for (lane = 0; lane < 4; ++lane) {
			clrbits_le32(&mctl_phy->dx[lane].gcr[2], 0xffff);
			clrbits_le32(&mctl_phy->dx[lane].gcr[3],
				     (0x3<<12) | (0x3<<4));
		}
	} else {
		/* TODO: check */
		int lane;
		for (lane = 0; lane < 4; ++lane) {
			clrsetbits_le32(&mctl_phy->dx[lane].gcr[2], 0xffff,
					0xaaaa);
			if (para->dram_type == DRAM_TYPE_DDR3)
				setbits_le32(&mctl_phy->dx[lane].gcr[3],
					     (0x3<<12) | (0x3<<4));
			else
				setbits_le32(&mctl_phy->dx[lane].gcr[3],
					     0x00000012);
		}
	}

	writel(0x04058D02, &mctl_phy->zq[0].cr); /* CK/CA */
	writel(0x04058D02, &mctl_phy->zq[1].cr); /* DX0/DX1 */
	writel(0x04058D02, &mctl_phy->zq[2].cr); /* DX2/DX3 */

	/* Disable auto-refresh prior to data training */
	setbits_le32(&mctl_ctl->rfshctl3, MCTL_RFSHCTL3_DIS_AUTO_REFRESH);

	setbits_le32(&mctl_phy->dsgcr, 0xf << 24); /* unclear what this is... */
	/* TODO: IODDRM (IO DDR-MODE) for DDR3L */
	clrsetbits_le32(&mctl_phy->pgcr[1],
			MCTL_PGCR1_ZCKSEL_MASK,
			MCTL_PGCR1_IODDRM_DDR3 | MCTL_PGCR1_INHVT_EN);

	setbits_le32(&mctl_phy->pllcr, 0x3 << 19); /* PLL frequency select */
	/* TODO: single-channel PLL mode??? missing */
	setbits_le32(&mctl_phy->pllcr,
		     MCTL_PLLGCR_PLL_BYPASS | MCTL_PLLGCR_PLL_POWERDOWN);
	/* setbits_le32(&mctl_phy->pir, MCTL_PIR_PLL_BYPASS); included below */

	/* Disable VT compensation */
	clrbits_le32(&mctl_phy->pgcr[0], 0x3f);

	/* TODO: "other" PLL mode ... 0x20000 seems to be the PLL Bypass */
	if (para->dram_type == DRAM_TYPE_DDR3)
		clrsetbits_le32(&mctl_phy->pir, MCTL_PIR_MASK, 0x20df3);
	else
		clrsetbits_le32(&mctl_phy->pir, MCTL_PIR_MASK, 0x2c573);

	sdelay(10000); /* XXX necessary? */

	/* Wait for the INIT bit to clear itself... */
	while ((readl(&mctl_phy->pir) & MCTL_PIR_INIT) != MCTL_PIR_INIT) {
		/* not done yet -- keep spinning */
		debug("MCTL_PIR_INIT not set\n");
		sdelay(1000);
		/* TODO: implement timeout */
	}

	/* TODO: not used --- there's a "2rank debug" section here */

	/* Original dram init code which may come in handy later
	********************************************************
	 * LPDDR2 and LPDDR3 *
	if ((para->dram_type) == 6 || (para->dram_type) == 7) {
		reg_val = mctl_read_w(P0_DSGCR + ch_offset);
		reg_val &= (~(0x3<<6));		* set DQSGX to 1 *
		reg_val |= (0x1<<6);		* dqs gate extend *
		mctl_write_w(P0_DSGCR + ch_offset, reg_val);
		dram_dbg("DQS Gate Extend Enable!\n", ch_index);
	}

	 * Disable ZCAL after initial--for nand dma debug--20140330 by YSZ *
	if (para->dram_tpr13 & (0x1<<31)) {
		reg_val = mctl_read_w(P0_ZQ0CR + ch_offset);
		reg_val |= (0x7<<11);
		mctl_write_w(P0_ZQ0CR + ch_offset, reg_val);
	}
	********************************************************
	*/

	/*
	 * TODO: more 2-rank support
	 * (setting the "dqs gate delay to average between 2 rank")
	 */

	/* check if any errors are set */
	if (readl(&mctl_phy->pgsr[0]) & MCTL_PGSR0_ERRORS) {
		debug("Channel %d unavailable!\n", ch_index);
		return 0;
	} else{
		/* initial OK */
		debug("Channel %d OK!\n", ch_index);
		/* return 1; */
	}

	while ((readl(&mctl_ctl->stat) & 0x1) != 0x1) {
		debug("Waiting for INIT to be done (controller to come up into 'normal operating' mode\n");
		sdelay(100000);
		/* init not done */
		/* TODO: implement time-out */
	}
	debug("done\n");

	/* "DDR is controller by contoller" */
	clrbits_le32(&mctl_phy->pgcr[3], (1 << 25));

	/* TODO: is the following necessary? */
	debug("DFIMISC before writing 0: 0x%x\n", readl(&mctl_ctl->dfimisc));
	writel(0, &mctl_ctl->dfimisc);

	/* Enable auto-refresh */
	clrbits_le32(&mctl_ctl->rfshctl3, MCTL_RFSHCTL3_DIS_AUTO_REFRESH);

	debug("channel_init complete\n");
	return 1;
}

signed int DRAMC_get_dram_size(void)
{
	struct sunxi_mctl_com_reg * const mctl_com =
		(struct sunxi_mctl_com_reg *)SUNXI_DRAM_COM_BASE;

	unsigned int reg_val;
	unsigned int dram_size;
	unsigned int temp;

	reg_val = readl(&mctl_com->cr);

	temp = (reg_val >> 8) & 0xf;	/* page size code */
	dram_size = (temp - 6);		/* (1 << dram_size) * 512Bytes */

	temp = (reg_val >> 4) & 0xf;	/* row width code */
	dram_size += (temp + 1);	/* (1 << dram_size) * 512Bytes */

	temp = (reg_val >> 2) & 0x3;	/* bank number code */
	dram_size += (temp + 2);	/* (1 << dram_size) * 512Bytes */

	temp = reg_val & 0x3;		/* rank number code */
	dram_size += temp;		/* (1 << dram_size) * 512Bytes */

	temp = (reg_val >> 19) & 0x1;	/* channel number code */
	dram_size += temp;		/* (1 << dram_size) * 512Bytes */

	dram_size = dram_size - 11;	/* (1 << dram_size) MBytes */

	return 1 << dram_size;
}

unsigned long sunxi_dram_init(void)
{
	struct sunxi_mctl_com_reg * const mctl_com =
		(struct sunxi_mctl_com_reg *)SUNXI_DRAM_COM_BASE;

	struct dram_sun9i_cl_cwl_timing cl_cwl[] = {
		{ .CL =  5, .CWL = 5, .tCKmin = 3000, .tCKmax = 3300 },
		{ .CL =  6, .CWL = 5, .tCKmin = 2500, .tCKmax = 3300 },
		{ .CL =  8, .CWL = 6, .tCKmin = 1875, .tCKmax = 2500 },
		{ .CL = 10, .CWL = 7, .tCKmin = 1500, .tCKmax = 1875 },
		{ .CL = 11, .CWL = 8, .tCKmin = 1250, .tCKmax = 1500 }
	};

	/* Set initial parameters, these get modified by the autodetect code */
	struct dram_sun9i_para para = {
		.dram_type = DRAM_TYPE_DDR3,
		.bus_width = 32,
		.chan = 2,
		.rank = 1,
		/* .rank = 2, */
		.page_size = 4096,
		/* .rows = 16, */
		.rows = 15,

		/* CL/CWL table for the speed bin */
		.cl_cwl_table = cl_cwl,
		.cl_cwl_numentries = sizeof(cl_cwl) /
			sizeof(struct dram_sun9i_cl_cwl_timing),

		/* timings */
		.tREFI = 7800,	/* 7.8us (up to 85 degC) */
		.tRFC  = 260,	/* 260ns for 4GBit devices */
				/* 350ns @ 8GBit */

		.tRCD  = 13750,
		.tRP   = 13750,
		.tRC   = 48750,
		.tRAS  = 35000,

		.tDLLK = 512,
		.tRTP  = { .ck = 4, .ps = 7500 },
		.tWTR  = { .ck = 4, .ps = 7500 },
		.tWR   = 15,
		.tMRD  = 4,
		.tMOD  = { .ck = 12, .ps = 15000 },
		.tCCD  = 4,
		.tRRD  = { .ck = 4, .ps = 7500 },
		.tFAW  = 40,

		/* calibration timing */
		/* .tZQinit = { .ck = 512, .ps = 640000 }, */
		.tZQoper = { .ck = 256, .ps = 320000 },
		.tZQCS   = { .ck = 64,  .ps = 80000 },

		/* reset timing */
		/* .tXPR  = { .ck = 5, .ps = 10000 }, */

		/* self-refresh timings */
		.tXS  = { .ck = 5, .ps = 10000 },
		.tXSDLL = 512,
		.tCKSRE = { .ck = 5, .ps = 10000 },
		.tCKSRX = { .ck = 5, .ps = 10000 },

		/* power-down timings */
		.tXP = { .ck = 3, .ps = 6000 },
		.tXPDLL = { .ck = 10, .ps = 24000 },
		.tCKE = { .ck = 3, .ps = 5000 },

		/* write leveling timings */
		.tWLMRD = 40,
		/* .tWLDQSEN = 25, */
		.tWLO = 7500,
		/* .tWLOE = 2000, */
	};

	/*
	 * Disable A80 internal 240 ohm resistor.
	 *
	 * This code sequence is adapated from Allwinner's Boot0 (see
	 * https://github.com/allwinner-zh/bootloader.git), as there
	 * is no documentation for these two registers in the R_PRCM
	 * block.
	 */
	setbits_le32(SUNXI_PRCM_BASE + 0x1e0, (0x3 << 8));
	writel(0, SUNXI_PRCM_BASE + 0x1e8);

	mctl_sys_init();

	if (!mctl_channel_init(0, &para))
		return 0;

	/* dual-channel */
	if (!mctl_channel_init(1, &para)) {
		/* disable channel 1 */
		clrsetbits_le32(&mctl_com->cr, MCTL_CR_CHANNEL_MASK,
				MCTL_CR_CHANNEL_SINGLE);
		/* disable channel 1 global clock */
		clrbits_le32(&mctl_com->cr, MCTL_CCR_CH1_CLK_EN);
	}

	mctl_com_init(&para);

	/* return the proper RAM size */
	return DRAMC_get_dram_size() << 20;
}
