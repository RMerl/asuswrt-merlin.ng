// SPDX-License-Identifier: GPL-2.0+
/*
 * Sun6i platform dram controller init.
 *
 * (C) Copyright 2007-2012
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Berg Xing <bergxing@allwinnertech.com>
 * Tom Cubie <tangliang@allwinnertech.com>
 *
 * (C) Copyright 2014 Hans de Goede <hdegoede@redhat.com>
 */
#include <common.h>
#include <errno.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/dram.h>
#include <asm/arch/prcm.h>

#define DRAM_CLK (CONFIG_DRAM_CLK * 1000000)

struct dram_sun6i_para {
	u8 bus_width;
	u8 chan;
	u8 rank;
	u8 rows;
	u16 page_size;
};

static void mctl_sys_init(void)
{
	struct sunxi_ccm_reg * const ccm =
		(struct sunxi_ccm_reg *)SUNXI_CCM_BASE;
	const int dram_clk_div = 2;

	clock_set_pll5(DRAM_CLK * dram_clk_div, false);

	clrsetbits_le32(&ccm->dram_clk_cfg, CCM_DRAMCLK_CFG_DIV0_MASK,
		CCM_DRAMCLK_CFG_DIV0(dram_clk_div) | CCM_DRAMCLK_CFG_RST |
		CCM_DRAMCLK_CFG_UPD);
	mctl_await_completion(&ccm->dram_clk_cfg, CCM_DRAMCLK_CFG_UPD, 0);

	writel(MDFS_CLK_DEFAULT, &ccm->mdfs_clk_cfg);

	/* deassert mctl reset */
	setbits_le32(&ccm->ahb_reset0_cfg, 1 << AHB_RESET_OFFSET_MCTL);

	/* enable mctl clock */
	setbits_le32(&ccm->ahb_gate0, 1 << AHB_GATE_OFFSET_MCTL);
}

static void mctl_dll_init(int ch_index, struct dram_sun6i_para *para)
{
	struct sunxi_mctl_phy_reg *mctl_phy;

	if (ch_index == 0)
		mctl_phy = (struct sunxi_mctl_phy_reg *)SUNXI_DRAM_PHY0_BASE;
	else
		mctl_phy = (struct sunxi_mctl_phy_reg *)SUNXI_DRAM_PHY1_BASE;

	/* disable + reset dlls */
	writel(MCTL_DLLCR_DISABLE, &mctl_phy->acdllcr);
	writel(MCTL_DLLCR_DISABLE, &mctl_phy->dx0dllcr);
	writel(MCTL_DLLCR_DISABLE, &mctl_phy->dx1dllcr);
	if (para->bus_width == 32) {
		writel(MCTL_DLLCR_DISABLE, &mctl_phy->dx2dllcr);
		writel(MCTL_DLLCR_DISABLE, &mctl_phy->dx3dllcr);
	}
	udelay(2);

	/* enable + reset dlls */
	writel(0, &mctl_phy->acdllcr);
	writel(0, &mctl_phy->dx0dllcr);
	writel(0, &mctl_phy->dx1dllcr);
	if (para->bus_width == 32) {
		writel(0, &mctl_phy->dx2dllcr);
		writel(0, &mctl_phy->dx3dllcr);
	}
	udelay(22);

	/* enable and release reset of dlls */
	writel(MCTL_DLLCR_NRESET, &mctl_phy->acdllcr);
	writel(MCTL_DLLCR_NRESET, &mctl_phy->dx0dllcr);
	writel(MCTL_DLLCR_NRESET, &mctl_phy->dx1dllcr);
	if (para->bus_width == 32) {
		writel(MCTL_DLLCR_NRESET, &mctl_phy->dx2dllcr);
		writel(MCTL_DLLCR_NRESET, &mctl_phy->dx3dllcr);
	}
	udelay(22);
}

static bool mctl_rank_detect(u32 *gsr0, int rank)
{
	const u32 done = MCTL_DX_GSR0_RANK0_TRAIN_DONE << rank;
	const u32 err = MCTL_DX_GSR0_RANK0_TRAIN_ERR << rank;

	mctl_await_completion(gsr0, done, done);
	mctl_await_completion(gsr0 + 0x10, done, done);

	return !(readl(gsr0) & err) && !(readl(gsr0 + 0x10) & err);
}

static void mctl_channel_init(int ch_index, struct dram_sun6i_para *para)
{
	struct sunxi_mctl_com_reg * const mctl_com =
		(struct sunxi_mctl_com_reg *)SUNXI_DRAM_COM_BASE;
	struct sunxi_mctl_ctl_reg *mctl_ctl;
	struct sunxi_mctl_phy_reg *mctl_phy;

	if (ch_index == 0) {
		mctl_ctl = (struct sunxi_mctl_ctl_reg *)SUNXI_DRAM_CTL0_BASE;
		mctl_phy = (struct sunxi_mctl_phy_reg *)SUNXI_DRAM_PHY0_BASE;
	} else {
		mctl_ctl = (struct sunxi_mctl_ctl_reg *)SUNXI_DRAM_CTL1_BASE;
		mctl_phy = (struct sunxi_mctl_phy_reg *)SUNXI_DRAM_PHY1_BASE;
	}

	writel(MCTL_MCMD_NOP, &mctl_ctl->mcmd);
	mctl_await_completion(&mctl_ctl->mcmd, MCTL_MCMD_BUSY, 0);

	/* PHY initialization */
	writel(MCTL_PGCR, &mctl_phy->pgcr);
	writel(MCTL_MR0, &mctl_phy->mr0);
	writel(MCTL_MR1, &mctl_phy->mr1);
	writel(MCTL_MR2, &mctl_phy->mr2);
	writel(MCTL_MR3, &mctl_phy->mr3);

	writel((MCTL_TITMSRST << 18) | (MCTL_TDLLLOCK << 6) | MCTL_TDLLSRST,
	       &mctl_phy->ptr0);

	writel((MCTL_TDINIT1 << 19) | MCTL_TDINIT0, &mctl_phy->ptr1);
	writel((MCTL_TDINIT3 << 17) | MCTL_TDINIT2, &mctl_phy->ptr2);

	writel((MCTL_TCCD << 31) | (MCTL_TRC << 25) | (MCTL_TRRD << 21) |
	       (MCTL_TRAS << 16) | (MCTL_TRCD << 12) | (MCTL_TRP << 8) |
	       (MCTL_TWTR << 5) | (MCTL_TRTP << 2) | (MCTL_TMRD << 0),
	       &mctl_phy->dtpr0);

	writel((MCTL_TDQSCKMAX << 27) | (MCTL_TDQSCK << 24) |
	       (MCTL_TRFC << 16) | (MCTL_TRTODT << 11) |
	       ((MCTL_TMOD - 12) << 9) | (MCTL_TFAW << 3) | (0 << 2) |
	       (MCTL_TAOND << 0), &mctl_phy->dtpr1);

	writel((MCTL_TDLLK << 19) | (MCTL_TCKE << 15) | (MCTL_TXPDLL << 10) |
	       (MCTL_TEXSR << 0), &mctl_phy->dtpr2);

	writel(1, &mctl_ctl->dfitphyupdtype0);
	writel(MCTL_DCR_DDR3, &mctl_phy->dcr);
	writel(MCTL_DSGCR, &mctl_phy->dsgcr);
	writel(MCTL_DXCCR, &mctl_phy->dxccr);
	writel(MCTL_DX_GCR | MCTL_DX_GCR_EN, &mctl_phy->dx0gcr);
	writel(MCTL_DX_GCR | MCTL_DX_GCR_EN, &mctl_phy->dx1gcr);
	writel(MCTL_DX_GCR | MCTL_DX_GCR_EN, &mctl_phy->dx2gcr);
	writel(MCTL_DX_GCR | MCTL_DX_GCR_EN, &mctl_phy->dx3gcr);

	mctl_await_completion(&mctl_phy->pgsr, 0x03, 0x03);

	writel(CONFIG_DRAM_ZQ, &mctl_phy->zq0cr1);

	setbits_le32(&mctl_phy->pir, MCTL_PIR_CLEAR_STATUS);
	writel(MCTL_PIR_STEP1, &mctl_phy->pir);
	udelay(10);
	mctl_await_completion(&mctl_phy->pgsr, 0x1f, 0x1f);

	/* rank detect */
	if (!mctl_rank_detect(&mctl_phy->dx0gsr0, 1)) {
		para->rank = 1;
		clrbits_le32(&mctl_phy->pgcr, MCTL_PGCR_RANK);
	}

	/*
	 * channel detect, check channel 1 dx0 and dx1 have rank 0, if not
	 * assume nothing is connected to channel 1.
	 */
	if (ch_index == 1 && !mctl_rank_detect(&mctl_phy->dx0gsr0, 0)) {
		para->chan = 1;
		clrbits_le32(&mctl_com->ccr, MCTL_CCR_CH1_CLK_EN);
		return;
	}

	/* bus width detect, if dx2 and dx3 don't have rank 0, assume 16 bit */
	if (!mctl_rank_detect(&mctl_phy->dx2gsr0, 0)) {
		para->bus_width = 16;
		para->page_size = 2048;
		setbits_le32(&mctl_phy->dx2dllcr, MCTL_DLLCR_DISABLE);
		setbits_le32(&mctl_phy->dx3dllcr, MCTL_DLLCR_DISABLE);
		clrbits_le32(&mctl_phy->dx2gcr, MCTL_DX_GCR_EN);
		clrbits_le32(&mctl_phy->dx3gcr, MCTL_DX_GCR_EN);
	}

	setbits_le32(&mctl_phy->pir, MCTL_PIR_CLEAR_STATUS);
	writel(MCTL_PIR_STEP2, &mctl_phy->pir);
	udelay(10);
	mctl_await_completion(&mctl_phy->pgsr, 0x11, 0x11);

	if (readl(&mctl_phy->pgsr) & MCTL_PGSR_TRAIN_ERR_MASK)
		panic("Training error initialising DRAM\n");

	/* Move to configure state */
	writel(MCTL_SCTL_CONFIG, &mctl_ctl->sctl);
	mctl_await_completion(&mctl_ctl->sstat, 0x07, 0x01);

	/* Set number of clks per micro-second */
	writel(DRAM_CLK / 1000000, &mctl_ctl->togcnt1u);
	/* Set number of clks per 100 nano-seconds */
	writel(DRAM_CLK / 10000000, &mctl_ctl->togcnt100n);
	/* Set memory timing registers */
	writel(MCTL_TREFI, &mctl_ctl->trefi);
	writel(MCTL_TMRD, &mctl_ctl->tmrd);
	writel(MCTL_TRFC, &mctl_ctl->trfc);
	writel((MCTL_TPREA << 16) | MCTL_TRP, &mctl_ctl->trp);
	writel(MCTL_TRTW, &mctl_ctl->trtw);
	writel(MCTL_TAL, &mctl_ctl->tal);
	writel(MCTL_TCL, &mctl_ctl->tcl);
	writel(MCTL_TCWL, &mctl_ctl->tcwl);
	writel(MCTL_TRAS, &mctl_ctl->tras);
	writel(MCTL_TRC, &mctl_ctl->trc);
	writel(MCTL_TRCD, &mctl_ctl->trcd);
	writel(MCTL_TRRD, &mctl_ctl->trrd);
	writel(MCTL_TRTP, &mctl_ctl->trtp);
	writel(MCTL_TWR, &mctl_ctl->twr);
	writel(MCTL_TWTR, &mctl_ctl->twtr);
	writel(MCTL_TEXSR, &mctl_ctl->texsr);
	writel(MCTL_TXP, &mctl_ctl->txp);
	writel(MCTL_TXPDLL, &mctl_ctl->txpdll);
	writel(MCTL_TZQCS, &mctl_ctl->tzqcs);
	writel(MCTL_TZQCSI, &mctl_ctl->tzqcsi);
	writel(MCTL_TDQS, &mctl_ctl->tdqs);
	writel(MCTL_TCKSRE, &mctl_ctl->tcksre);
	writel(MCTL_TCKSRX, &mctl_ctl->tcksrx);
	writel(MCTL_TCKE, &mctl_ctl->tcke);
	writel(MCTL_TMOD, &mctl_ctl->tmod);
	writel(MCTL_TRSTL, &mctl_ctl->trstl);
	writel(MCTL_TZQCL, &mctl_ctl->tzqcl);
	writel(MCTL_TMRR, &mctl_ctl->tmrr);
	writel(MCTL_TCKESR, &mctl_ctl->tckesr);
	writel(MCTL_TDPD, &mctl_ctl->tdpd);

	/* Unknown magic performed by boot0 */
	setbits_le32(&mctl_ctl->dfiodtcfg, 1 << 3);
	clrbits_le32(&mctl_ctl->dfiodtcfg1, 0x1f);

	/* Select 16/32-bits mode for MCTL */
	if (para->bus_width == 16)
		setbits_le32(&mctl_ctl->ppcfg, 1);

	/* Set DFI timing registers */
	writel(MCTL_TCWL, &mctl_ctl->dfitphywrl);
	writel(MCTL_TCL - 1, &mctl_ctl->dfitrdden);
	writel(MCTL_DFITPHYRDL, &mctl_ctl->dfitphyrdl);
	writel(MCTL_DFISTCFG0, &mctl_ctl->dfistcfg0);

	writel(MCTL_MCFG_DDR3, &mctl_ctl->mcfg);

	/* DFI update configuration register */
	writel(MCTL_DFIUPDCFG_UPD, &mctl_ctl->dfiupdcfg);

	/* Move to access state */
	writel(MCTL_SCTL_ACCESS, &mctl_ctl->sctl);
	mctl_await_completion(&mctl_ctl->sstat, 0x07, 0x03);
}

static void mctl_com_init(struct dram_sun6i_para *para)
{
	struct sunxi_mctl_com_reg * const mctl_com =
		(struct sunxi_mctl_com_reg *)SUNXI_DRAM_COM_BASE;
	struct sunxi_mctl_phy_reg * const mctl_phy1 =
		(struct sunxi_mctl_phy_reg *)SUNXI_DRAM_PHY1_BASE;
	struct sunxi_prcm_reg * const prcm =
		(struct sunxi_prcm_reg *)SUNXI_PRCM_BASE;

	writel(MCTL_CR_UNKNOWN | MCTL_CR_CHANNEL(para->chan) | MCTL_CR_DDR3 |
	       ((para->bus_width == 32) ? MCTL_CR_BUSW32 : MCTL_CR_BUSW16) |
	       MCTL_CR_PAGE_SIZE(para->page_size) | MCTL_CR_ROW(para->rows) |
	       MCTL_CR_BANK(1) | MCTL_CR_RANK(para->rank), &mctl_com->cr);

	/* Unknown magic performed by boot0 */
	setbits_le32(&mctl_com->dbgcr, (1 << 6));

	if (para->chan == 1) {
		/* Shutdown channel 1 */
		setbits_le32(&mctl_phy1->aciocr, MCTL_ACIOCR_DISABLE);
		setbits_le32(&mctl_phy1->dxccr, MCTL_DXCCR_DISABLE);
		clrbits_le32(&mctl_phy1->dsgcr, MCTL_DSGCR_ENABLE);
		/*
		 * CH0 ?? this is what boot0 does. Leave as is until we can
		 * confirm this.
		 */
		setbits_le32(&prcm->vdd_sys_pwroff,
			     PRCM_VDD_SYS_DRAM_CH0_PAD_HOLD_PWROFF);
	}
}

static void mctl_port_cfg(void)
{
	struct sunxi_mctl_com_reg * const mctl_com =
		(struct sunxi_mctl_com_reg *)SUNXI_DRAM_COM_BASE;
	struct sunxi_ccm_reg * const ccm =
		(struct sunxi_ccm_reg *)SUNXI_CCM_BASE;

	/* enable DRAM AXI clock for CPU access */
	setbits_le32(&ccm->axi_gate, 1 << AXI_GATE_OFFSET_DRAM);

	/* Bunch of magic writes performed by boot0 */
	writel(0x00400302, &mctl_com->rmcr[0]);
	writel(0x01000307, &mctl_com->rmcr[1]);
	writel(0x00400302, &mctl_com->rmcr[2]);
	writel(0x01000307, &mctl_com->rmcr[3]);
	writel(0x01000307, &mctl_com->rmcr[4]);
	writel(0x01000303, &mctl_com->rmcr[6]);
	writel(0x01000303, &mctl_com->mmcr[0]);
	writel(0x00400310, &mctl_com->mmcr[1]);
	writel(0x01000307, &mctl_com->mmcr[2]);
	writel(0x01000303, &mctl_com->mmcr[3]);
	writel(0x01800303, &mctl_com->mmcr[4]);
	writel(0x01800303, &mctl_com->mmcr[5]);
	writel(0x01800303, &mctl_com->mmcr[6]);
	writel(0x01800303, &mctl_com->mmcr[7]);
	writel(0x01000303, &mctl_com->mmcr[8]);
	writel(0x00000002, &mctl_com->mmcr[15]);
	writel(0x00000310, &mctl_com->mbagcr[0]);
	writel(0x00400310, &mctl_com->mbagcr[1]);
	writel(0x00400310, &mctl_com->mbagcr[2]);
	writel(0x00000307, &mctl_com->mbagcr[3]);
	writel(0x00000317, &mctl_com->mbagcr[4]);
	writel(0x00000307, &mctl_com->mbagcr[5]);
}

unsigned long sunxi_dram_init(void)
{
	struct sunxi_mctl_com_reg * const mctl_com =
		(struct sunxi_mctl_com_reg *)SUNXI_DRAM_COM_BASE;
	u32 offset;
	int bank, bus, columns;

	/* Set initial parameters, these get modified by the autodetect code */
	struct dram_sun6i_para para = {
		.bus_width = 32,
		.chan = 2,
		.rank = 2,
		.page_size = 4096,
		.rows = 16,
	};

	/* A31s only has one channel */
	if (sunxi_get_ss_bonding_id() == SUNXI_SS_BOND_ID_A31S)
		para.chan = 1;

	mctl_sys_init();

	mctl_dll_init(0, &para);
	setbits_le32(&mctl_com->ccr, MCTL_CCR_CH0_CLK_EN);

	if (para.chan == 2) {
		mctl_dll_init(1, &para);
		setbits_le32(&mctl_com->ccr, MCTL_CCR_CH1_CLK_EN);
	}

	setbits_le32(&mctl_com->ccr, MCTL_CCR_MASTER_CLK_EN);

	mctl_channel_init(0, &para);
	if (para.chan == 2)
		mctl_channel_init(1, &para);

	mctl_com_init(&para);
	mctl_port_cfg();

	/*
	 * Change to 1 ch / sequence / 8192 byte pages / 16 rows /
	 * 8 bit banks / 1 rank mode.
	 */
	clrsetbits_le32(&mctl_com->cr,
		MCTL_CR_CHANNEL_MASK | MCTL_CR_PAGE_SIZE_MASK |
		    MCTL_CR_ROW_MASK | MCTL_CR_BANK_MASK | MCTL_CR_RANK_MASK,
		MCTL_CR_CHANNEL(1) | MCTL_CR_SEQUENCE |
		    MCTL_CR_PAGE_SIZE(8192) | MCTL_CR_ROW(16) |
		    MCTL_CR_BANK(1) | MCTL_CR_RANK(1));

	/* Detect and set page size */
	for (columns = 7; columns < 20; columns++) {
		if (mctl_mem_matches(1 << columns))
			break;
	}
	bus = (para.bus_width == 32) ? 2 : 1;
	columns -= bus;
	para.page_size = (1 << columns) * (bus << 1);
	clrsetbits_le32(&mctl_com->cr, MCTL_CR_PAGE_SIZE_MASK,
			MCTL_CR_PAGE_SIZE(para.page_size));

	/* Detect and set rows */
	for (para.rows = 11; para.rows < 16; para.rows++) {
		offset = 1 << (para.rows + columns + bus);
		if (mctl_mem_matches(offset))
			break;
	}
	clrsetbits_le32(&mctl_com->cr, MCTL_CR_ROW_MASK,
			MCTL_CR_ROW(para.rows));

	/* Detect bank size */
	offset = 1 << (para.rows + columns + bus + 2);
	bank = mctl_mem_matches(offset) ? 0 : 1;

	/* Restore interleave, chan and rank values, set bank size */
	clrsetbits_le32(&mctl_com->cr,
			MCTL_CR_CHANNEL_MASK | MCTL_CR_SEQUENCE |
			    MCTL_CR_BANK_MASK | MCTL_CR_RANK_MASK,
			MCTL_CR_CHANNEL(para.chan) | MCTL_CR_BANK(bank) |
			    MCTL_CR_RANK(para.rank));

	return 1 << (para.rank + para.rows + bank + columns + para.chan + bus);
}
