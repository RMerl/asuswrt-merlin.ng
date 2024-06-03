// SPDX-License-Identifier: GPL-2.0+
/*
 * (c) 2015 Paul Thacker <paul.thacker@microchip.com>
 *
 */
#include <common.h>
#include <wait_bit.h>
#include <linux/kernel.h>
#include <linux/bitops.h>
#include <mach/pic32.h>
#include <mach/ddr.h>

#include "ddr2_regs.h"
#include "ddr2_timing.h"

/* init DDR2 Phy */
void ddr2_phy_init(void)
{
	struct ddr2_phy_regs *ddr2_phy;
	u32 pad_ctl;

	ddr2_phy = ioremap(PIC32_DDR2P_BASE, sizeof(*ddr2_phy));

	/* PHY_DLL_RECALIB */
	writel(DELAY_START_VAL(3) | DISABLE_RECALIB(0) |
	       RECALIB_CNT(0x10), &ddr2_phy->dll_recalib);

	/* PHY_PAD_CTRL */
	pad_ctl = ODT_SEL | ODT_EN | DRIVE_SEL(0) |
		  ODT_PULLDOWN(2) | ODT_PULLUP(3) |
		  EXTRA_OEN_CLK(0) | NOEXT_DLL |
		  DLR_DFT_WRCMD | HALF_RATE |
		  DRVSTR_PFET(0xe) | DRVSTR_NFET(0xe) |
		  RCVR_EN | PREAMBLE_DLY(2);
	writel(pad_ctl, &ddr2_phy->pad_ctrl);

	/* SCL_CONFIG_0 */
	writel(SCL_BURST8 | SCL_DDR_CONNECTED | SCL_RCAS_LAT(RL) |
	       SCL_ODTCSWW, &ddr2_phy->scl_config_1);

	/* SCL_CONFIG_1 */
	writel(SCL_CSEN | SCL_WCAS_LAT(WL), &ddr2_phy->scl_config_2);

	/* SCL_LAT */
	writel(SCL_CAPCLKDLY(3) | SCL_DDRCLKDLY(4), &ddr2_phy->scl_latency);
}

/* start phy self calibration logic */
static int ddr2_phy_calib_start(void)
{
	struct ddr2_phy_regs *ddr2_phy;

	ddr2_phy = ioremap(PIC32_DDR2P_BASE, sizeof(*ddr2_phy));

	/* DDR Phy SCL Start */
	writel(SCL_START | SCL_EN, &ddr2_phy->scl_start);

	/* Wait for SCL for data byte to pass */
	return wait_for_bit_le32(&ddr2_phy->scl_start, SCL_LUBPASS,
				 true, CONFIG_SYS_HZ, false);
}

/* DDR2 Controller initialization */

/* Target Agent Arbiter */
static void ddr_set_arbiter(struct ddr2_ctrl_regs *ctrl,
			    const struct ddr2_arbiter_params *const param)
{
	int i;

	for (i = 0; i < NUM_AGENTS; i++) {
		/* set min burst size */
		writel(i * MIN_LIM_WIDTH, &ctrl->tsel);
		writel(param->min_limit, &ctrl->minlim);

		/* set request period (4 * req_period clocks) */
		writel(i * RQST_PERIOD_WIDTH, &ctrl->tsel);
		writel(param->req_period, &ctrl->reqprd);

		/* set number of burst accepted */
		writel(i * MIN_CMDACPT_WIDTH, &ctrl->tsel);
		writel(param->min_cmd_acpt, &ctrl->mincmd);
	}
}

const struct ddr2_arbiter_params *__weak board_get_ddr_arbiter_params(void)
{
	/* default arbiter parameters */
	static const struct ddr2_arbiter_params arb_params[] = {
		{ .min_limit = 0x1f, .req_period = 0xff, .min_cmd_acpt = 0x04,},
		{ .min_limit = 0x1f, .req_period = 0xff, .min_cmd_acpt = 0x10,},
		{ .min_limit = 0x1f, .req_period = 0xff, .min_cmd_acpt = 0x10,},
		{ .min_limit = 0x04, .req_period = 0xff, .min_cmd_acpt = 0x04,},
		{ .min_limit = 0x04, .req_period = 0xff, .min_cmd_acpt = 0x04,},
	};

	return &arb_params[0];
}

static void host_load_cmd(struct ddr2_ctrl_regs *ctrl, u32 cmd_idx,
			  u32 hostcmd2, u32 hostcmd1, u32 delay)
{
	u32 hc_delay;

	hc_delay = max_t(u32, DIV_ROUND_UP(delay, T_CK), 2) - 2;
	writel(hostcmd1, &ctrl->cmd10[cmd_idx]);
	writel((hostcmd2 & 0x7ff) | (hc_delay << 11), &ctrl->cmd20[cmd_idx]);
}

/* init DDR2 Controller */
void ddr2_ctrl_init(void)
{
	u32 wr2prech, rd2prech, wr2rd, wr2rd_cs;
	u32 ras2ras, ras2cas, prech2ras, temp;
	const struct ddr2_arbiter_params *arb_params;
	struct ddr2_ctrl_regs *ctrl;

	ctrl = ioremap(PIC32_DDR2C_BASE, sizeof(*ctrl));

	/* PIC32 DDR2 controller always work in HALF_RATE */
	writel(HALF_RATE_MODE, &ctrl->memwidth);

	/* Set arbiter configuration per target */
	arb_params = board_get_ddr_arbiter_params();
	ddr_set_arbiter(ctrl, arb_params);

	/* Address Configuration, model {CS, ROW, BA, COL} */
	writel((ROW_ADDR_RSHIFT | (BA_RSHFT << 8) | (CS_ADDR_RSHIFT << 16) |
	       (COL_HI_RSHFT << 24) | (SB_PRI << 29)  |
	       (EN_AUTO_PRECH << 30)), &ctrl->memcfg0);

	writel(ROW_ADDR_MASK, &ctrl->memcfg1);
	writel(COL_HI_MASK, &ctrl->memcfg2);
	writel(COL_LO_MASK, &ctrl->memcfg3);
	writel(BA_MASK | (CS_ADDR_MASK << 8), &ctrl->memcfg4);

	/* Refresh Config */
	writel(REFCNT_CLK(DIV_ROUND_UP(T_RFI, T_CK_CTRL) - 2) |
	       REFDLY_CLK(DIV_ROUND_UP(T_RFC_MIN, T_CK_CTRL) - 2) |
	       MAX_PEND_REF(7),
	       &ctrl->refcfg);

	/* Power Config */
	writel(ECC_EN(0) | ERR_CORR_EN(0) | EN_AUTO_PWR_DN(0) |
	       EN_AUTO_SELF_REF(3) | PWR_DN_DLY(8) |
	       SELF_REF_DLY(17) | PRECH_PWR_DN_ONLY(0),
	       &ctrl->pwrcfg);

	/* Delay Config */
	wr2rd = max_t(u32, DIV_ROUND_UP(T_WTR, T_CK_CTRL),
		      DIV_ROUND_UP(T_WTR_TCK, 2)) + WL + BL;
	wr2rd_cs = max_t(u32, wr2rd - 1, 3);
	wr2prech = DIV_ROUND_UP(T_WR, T_CK_CTRL) + WL + BL;
	rd2prech = max_t(u32, DIV_ROUND_UP(T_RTP, T_CK_CTRL),
			 DIV_ROUND_UP(T_RTP_TCK, 2)) + BL - 2;
	ras2ras = max_t(u32, DIV_ROUND_UP(T_RRD, T_CK_CTRL),
			DIV_ROUND_UP(T_RRD_TCK, 2)) - 1;
	ras2cas = DIV_ROUND_UP(T_RCD, T_CK_CTRL) - 1;
	prech2ras = DIV_ROUND_UP(T_RP, T_CK_CTRL) - 1;

	writel(((wr2rd & 0x0f) |
	       ((wr2rd_cs & 0x0f) << 4) |
	       ((BL - 1) << 8) |
	       (BL << 12) |
	       ((BL - 1) << 16) |
	       ((BL - 1) << 20) |
	       ((BL + 2) << 24) |
	       ((RL - WL + 3) << 28)), &ctrl->dlycfg0);

	writel(((T_CKE_TCK - 1) |
	       (((DIV_ROUND_UP(T_DLLK, 2) - 2) & 0xff) << 8) |
	       ((T_CKE_TCK - 1) << 16) |
	       ((max_t(u32, T_XP_TCK, T_CKE_TCK) - 1) << 20) |
	       ((wr2prech >> 4) << 26) |
	       ((wr2rd >> 4) << 27) |
	       ((wr2rd_cs >> 4) << 28) |
	       (((RL + 5) >> 4) << 29) |
	       ((DIV_ROUND_UP(T_DLLK, 2) >> 8) << 30)), &ctrl->dlycfg1);

	writel((DIV_ROUND_UP(T_RP, T_CK_CTRL) |
	       (rd2prech << 8) |
	       ((wr2prech & 0x0f) << 12) |
	       (ras2ras << 16) |
	       (ras2cas << 20) |
	       (prech2ras << 24) |
	       ((RL + 3) << 28)), &ctrl->dlycfg2);

	writel(((DIV_ROUND_UP(T_RAS_MIN, T_CK_CTRL) - 1) |
	       ((DIV_ROUND_UP(T_RC, T_CK_CTRL) - 1) << 8) |
	       ((DIV_ROUND_UP(T_FAW, T_CK_CTRL) - 1) << 16)),
	       &ctrl->dlycfg3);

	/* ODT Config */
	writel(0x0, &ctrl->odtcfg);
	writel(BIT(16), &ctrl->odtencfg);
	writel(ODTRDLY(RL - 3) | ODTWDLY(WL - 3) | ODTRLEN(2) | ODTWLEN(3),
	       &ctrl->odtcfg);

	/* Transfer Configuration */
	writel(NXTDATRQDLY(2) | NXDATAVDLY(4) | RDATENDLY(2) |
	       MAX_BURST(3) | (7 << 28) | BIG_ENDIAN(0),
	       &ctrl->xfercfg);

	/* DRAM Initialization */
	/* CKE high after reset and wait 400 nsec */
	host_load_cmd(ctrl, 0, 0, IDLE_NOP, 400000);

	/* issue precharge all command */
	host_load_cmd(ctrl, 1, 0x04, PRECH_ALL_CMD, T_RP + T_CK);

	/* initialize EMR2 */
	host_load_cmd(ctrl, 2, 0x200, LOAD_MODE_CMD, T_MRD_TCK * T_CK);

	/* initialize EMR3 */
	host_load_cmd(ctrl, 3, 0x300, LOAD_MODE_CMD, T_MRD_TCK * T_CK);

	/*
	 * RDQS disable, DQSB enable, OCD exit, 150 ohm termination,
	 * AL=0, DLL enable
	 */
	host_load_cmd(ctrl, 4, 0x100,
		      LOAD_MODE_CMD | (0x40 << 24), T_MRD_TCK * T_CK);
	/*
	 * PD fast exit, WR REC = T_WR in clocks -1,
	 * DLL reset, CAS = RL, burst = 4
	 */
	temp = ((DIV_ROUND_UP(T_WR, T_CK) - 1) << 1) | 1;
	host_load_cmd(ctrl, 5, temp, LOAD_MODE_CMD | (RL << 28) | (2 << 24),
		      T_MRD_TCK * T_CK);

	/* issue precharge all command */
	host_load_cmd(ctrl, 6, 4, PRECH_ALL_CMD, T_RP + T_CK);

	/* issue refresh command */
	host_load_cmd(ctrl, 7, 0, REF_CMD, T_RFC_MIN);

	/* issue refresh command */
	host_load_cmd(ctrl, 8, 0, REF_CMD, T_RFC_MIN);

	/* Mode register programming as before without DLL reset */
	host_load_cmd(ctrl, 9, temp, LOAD_MODE_CMD | (RL << 28) | (3 << 24),
		      T_MRD_TCK * T_CK);

	/* extended mode register same as before with OCD default */
	host_load_cmd(ctrl, 10, 0x103, LOAD_MODE_CMD | (0xc << 24),
		      T_MRD_TCK * T_CK);

	/* extended mode register same as before with OCD exit */
	host_load_cmd(ctrl, 11, 0x100, LOAD_MODE_CMD | (0x4 << 28),
		      140 * T_CK);

	writel(CMD_VALID | NUMHOSTCMD(11), &ctrl->cmdissue);

	/* start memory initialization */
	writel(INIT_START, &ctrl->memcon);

	/* wait for all host cmds to be transmitted */
	wait_for_bit_le32(&ctrl->cmdissue, CMD_VALID, false,
			  CONFIG_SYS_HZ, false);

	/* inform all cmds issued, ready for normal operation */
	writel(INIT_START | INIT_DONE, &ctrl->memcon);

	/* perform phy caliberation */
	if (ddr2_phy_calib_start())
		printf("ddr2: phy calib failed\n");
}

phys_size_t ddr2_calculate_size(void)
{
	u32 temp;

	temp = 1 << (COL_BITS + BA_BITS + ROW_BITS);
	/* 16-bit data width between controller and DIMM */
	temp = temp * CS_BITS * (16 / 8);
	return (phys_size_t)temp;
}
