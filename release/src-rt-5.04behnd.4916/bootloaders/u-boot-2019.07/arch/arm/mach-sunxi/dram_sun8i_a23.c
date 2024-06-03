// SPDX-License-Identifier: GPL-2.0+
/*
 * Sun8i platform dram controller init.
 *
 * (C) Copyright 2014 Hans de Goede <hdegoede@redhat.com>
 */

/*
 * Note this code uses a lot of magic hex values, that is because this code
 * simply replays the init sequence as done by the Allwinner boot0 code, so
 * we do not know what these values mean. There are no symbolic constants for
 * these magic values, since we do not know how to name them and making up
 * names for them is not useful.
 *
 * The register-layout of the sunxi_mctl_phy_reg-s looks a lot like the one
 * found in the TI Keystone2 documentation:
 * http://www.ti.com/lit/ug/spruhn7a/spruhn7a.pdf
 * "Table4-2 DDR3 PHY Registers"
 * This may be used as a (possible) reference for future work / cleanups.
 */

#include <common.h>
#include <errno.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/dram.h>
#include <asm/arch/prcm.h>

static const struct dram_para dram_para = {
	.clock = CONFIG_DRAM_CLK,
	.type = 3,
	.zq = CONFIG_DRAM_ZQ,
	.odt_en = IS_ENABLED(CONFIG_DRAM_ODT_EN),
	.odt_correction = CONFIG_DRAM_ODT_CORRECTION,
	.para1 = 0, /* not used (only used when tpr13 bit 31 is set */
	.para2 = 0, /* not used (only used when tpr13 bit 31 is set */
	.mr0 = 6736,
	.mr1 = 4,
	.mr2 = 16,
	.mr3 = 0,
	/* tpr0 - 10 contain timing constants or-ed together in u32 vals */
	.tpr0 = 0x2ab83def,
	.tpr1 = 0x18082356,
	.tpr2 = 0x00034156,
	.tpr3 = 0x448c5533,
	.tpr4 = 0x08010d00,
	.tpr5 = 0x0340b20f,
	.tpr6 = 0x20d118cc,
	.tpr7 = 0x14062485,
	.tpr8 = 0x220d1d52,
	.tpr9 = 0x1e078c22,
	.tpr10 = 0x3c,
	.tpr11 = 0, /* not used */
	.tpr12 = 0, /* not used */
	.tpr13 = 0x30000,
};

static void mctl_sys_init(void)
{
	struct sunxi_ccm_reg * const ccm =
		(struct sunxi_ccm_reg *)SUNXI_CCM_BASE;

	/* enable pll5, note the divide by 2 is deliberate! */
	clock_set_pll5(dram_para.clock * 1000000 / 2,
		       dram_para.tpr13 & 0x40000);

	/* deassert ahb mctl reset */
	setbits_le32(&ccm->ahb_reset0_cfg, 1 << AHB_RESET_OFFSET_MCTL);

	/* enable ahb mctl clock */
	setbits_le32(&ccm->ahb_gate0, 1 << AHB_GATE_OFFSET_MCTL);
}

static void mctl_apply_odt_correction(u32 *reg, int correction)
{
	int val;

	val = (readl(reg) >> 8) & 0xff;
	val += correction;

	/* clamp */
	if (val < 0)
		val = 0;
	else if (val > 255)
		val = 255;

	clrsetbits_le32(reg, 0xff00, val << 8);
}

static void mctl_init(u32 *bus_width)
{
	struct sunxi_ccm_reg * const ccm =
		(struct sunxi_ccm_reg *)SUNXI_CCM_BASE;
	struct sunxi_mctl_com_reg * const mctl_com =
		(struct sunxi_mctl_com_reg *)SUNXI_DRAM_COM_BASE;
	struct sunxi_mctl_ctl_reg * const mctl_ctl =
		(struct sunxi_mctl_ctl_reg *)SUNXI_DRAM_CTL0_BASE;
	struct sunxi_mctl_phy_reg * const mctl_phy =
		(struct sunxi_mctl_phy_reg *)SUNXI_DRAM_PHY0_BASE;

	if (dram_para.tpr13 & 0x20)
		writel(0x40b, &mctl_phy->dcr);
	else
		writel(0x1000040b, &mctl_phy->dcr);

	if (dram_para.clock >= 480)
		writel(0x5c000, &mctl_phy->dllgcr);
	else
		writel(0xdc000, &mctl_phy->dllgcr);

	writel(0x0a003e3f, &mctl_phy->pgcr0);
	writel(0x03008421, &mctl_phy->pgcr1);

	writel(dram_para.mr0, &mctl_phy->mr0);
	writel(dram_para.mr1, &mctl_phy->mr1);
	writel(dram_para.mr2, &mctl_phy->mr2);
	writel(dram_para.mr3, &mctl_phy->mr3);

	if (!(dram_para.tpr13 & 0x10000)) {
		clrsetbits_le32(&mctl_phy->dx0gcr, 0x3800, 0x2000);
		clrsetbits_le32(&mctl_phy->dx1gcr, 0x3800, 0x2000);
	}

	/*
	 * All the masking and shifting below converts what I assume are DDR
	 * timing constants from Allwinner dram_para tpr format to the actual
	 * timing registers format.
	 */

	writel((dram_para.tpr0 & 0x000fffff), &mctl_phy->ptr2);
	writel((dram_para.tpr1 & 0x1fffffff), &mctl_phy->ptr3);
	writel((dram_para.tpr0 & 0x3ff00000) >> 2 |
	       (dram_para.tpr2 & 0x0003ffff), &mctl_phy->ptr4);

	writel(dram_para.tpr3, &mctl_phy->dtpr0);
	writel(dram_para.tpr4, &mctl_phy->dtpr2);

	writel(0x01000081, &mctl_phy->dtcr);

	if (dram_para.clock <= 240 || !dram_para.odt_en) {
		clrbits_le32(&mctl_phy->dx0gcr, 0x600);
		clrbits_le32(&mctl_phy->dx1gcr, 0x600);
	}
	if (dram_para.clock <= 240) {
		writel(0, &mctl_phy->odtcr);
		writel(0, &mctl_ctl->odtmap);
	}

	writel(((dram_para.tpr5 & 0x0f00) << 12) |
	       ((dram_para.tpr5 & 0x00f8) <<  9) |
	       ((dram_para.tpr5 & 0x0007) <<  8),
	       &mctl_ctl->rfshctl0);

	writel(((dram_para.tpr5 & 0x0003f000) << 12) |
	       ((dram_para.tpr5 & 0x00fc0000) >>  2) |
	       ((dram_para.tpr5 & 0x3f000000) >> 16) |
	       ((dram_para.tpr6 & 0x0000003f) >>  0),
	       &mctl_ctl->dramtmg0);

	writel(((dram_para.tpr6 & 0x000007c0) << 10) |
	       ((dram_para.tpr6 & 0x0000f800) >> 3) |
	       ((dram_para.tpr6 & 0x003f0000) >> 16),
	       &mctl_ctl->dramtmg1);

	writel(((dram_para.tpr6 & 0x0fc00000) << 2) |
	       ((dram_para.tpr7 & 0x0000001f) << 16) |
	       ((dram_para.tpr7 & 0x000003e0) << 3) |
	       ((dram_para.tpr7 & 0x0000fc00) >> 10),
	       &mctl_ctl->dramtmg2);

	writel(((dram_para.tpr7 & 0x03ff0000) >> 16) |
	       ((dram_para.tpr6 & 0xf0000000) >> 16),
	       &mctl_ctl->dramtmg3);

	writel(((dram_para.tpr7 & 0x3c000000) >> 2 ) |
	       ((dram_para.tpr8 & 0x00000007) << 16) |
	       ((dram_para.tpr8 & 0x00000038) << 5) |
	       ((dram_para.tpr8 & 0x000003c0) >> 6),
	       &mctl_ctl->dramtmg4);

	writel(((dram_para.tpr8 & 0x00003c00) << 14) |
	       ((dram_para.tpr8 & 0x0003c000) <<  2) |
	       ((dram_para.tpr8 & 0x00fc0000) >> 10) |
	       ((dram_para.tpr8 & 0x0f000000) >> 24),
	       &mctl_ctl->dramtmg5);

	writel(0x00000008, &mctl_ctl->dramtmg8);

	writel(((dram_para.tpr8 & 0xf0000000) >> 4) |
	       ((dram_para.tpr9 & 0x00007c00) << 6) |
	       ((dram_para.tpr9 & 0x000003e0) << 3) |
	       ((dram_para.tpr9 & 0x0000001f) >> 0),
	       &mctl_ctl->pitmg0);

	setbits_le32(&mctl_ctl->pitmg1, 0x80000);

	writel(((dram_para.tpr9 & 0x003f8000) << 9) | 0x2001,
	       &mctl_ctl->sched);

	writel((dram_para.mr0 << 16) | dram_para.mr1, &mctl_ctl->init3);
	writel((dram_para.mr2 << 16) | dram_para.mr3, &mctl_ctl->init4);

	writel(0x00000000, &mctl_ctl->pimisc);
	writel(0x80000000, &mctl_ctl->upd0);

	writel(((dram_para.tpr9  & 0xffc00000) >> 22) |
	       ((dram_para.tpr10 & 0x00000fff) << 16),
	       &mctl_ctl->rfshtmg);

	if (dram_para.tpr13 & 0x20)
		writel(0x01040001, &mctl_ctl->mstr);
	else
		writel(0x01040401, &mctl_ctl->mstr);

	if (!(dram_para.tpr13 & 0x20000)) {
		writel(0x00000002, &mctl_ctl->pwrctl);
		writel(0x00008001, &mctl_ctl->pwrtmg);
	}

	writel(0x00000001, &mctl_ctl->rfshctl3);
	writel(0x00000001, &mctl_ctl->pimisc);

	/* deassert dram_clk_cfg reset */
	setbits_le32(&ccm->dram_clk_cfg, CCM_DRAMCLK_CFG_RST);

	setbits_le32(&mctl_com->ccr, 0x80000);

	/* zq stuff */
	writel((dram_para.zq >> 8) & 0xff, &mctl_phy->zqcr1);

	writel(0x00000003, &mctl_phy->pir);
	udelay(10);
	mctl_await_completion(&mctl_phy->pgsr0, 0x09, 0x09);

	writel(readl(&mctl_phy->zqsr0) | 0x10000000, &mctl_phy->zqcr2);
	writel(dram_para.zq & 0xff, &mctl_phy->zqcr1);

	/* A23-v1.0 SDK uses 0xfdf3, A23-v2.0 SDK uses 0x5f3 */
	writel(0x000005f3, &mctl_phy->pir);
	udelay(10);
	mctl_await_completion(&mctl_phy->pgsr0, 0x03, 0x03);

	if (readl(&mctl_phy->dx1gsr0) & 0x1000000) {
		*bus_width = 8;
		writel(0, &mctl_phy->dx1gcr);
		writel(dram_para.zq & 0xff, &mctl_phy->zqcr1);
		writel(0x5f3, &mctl_phy->pir);
		udelay(10000);
		setbits_le32(&mctl_ctl->mstr, 0x1000);
	} else
		*bus_width = 16;

	if (dram_para.odt_correction) {
		mctl_apply_odt_correction(&mctl_phy->dx0lcdlr1,
					  dram_para.odt_correction);
		mctl_apply_odt_correction(&mctl_phy->dx1lcdlr1,
					  dram_para.odt_correction);
	}

	mctl_await_completion(&mctl_ctl->statr, 0x01, 0x01);

	writel(0x08003e3f, &mctl_phy->pgcr0);
	writel(0x00000000, &mctl_ctl->rfshctl3);
}

unsigned long sunxi_dram_init(void)
{
	struct sunxi_mctl_com_reg * const mctl_com =
		(struct sunxi_mctl_com_reg *)SUNXI_DRAM_COM_BASE;
	const u32 columns = 13;
	u32 bus, bus_width, offset, page_size, rows;

	mctl_sys_init();
	mctl_init(&bus_width);

	if (bus_width == 16) {
		page_size = 8;
		bus = 1;
	} else {
		page_size = 7;
		bus = 0;
	}

	if (!(dram_para.tpr13 & 0x80000000)) {
		/* Detect and set rows */
		writel(0x000310f4 | MCTL_CR_PAGE_SIZE(page_size),
		       &mctl_com->cr);
		setbits_le32(&mctl_com->swonr, 0x0003ffff);
		for (rows = 11; rows < 16; rows++) {
			offset = 1 << (rows + columns + bus);
			if (mctl_mem_matches(offset))
				break;
		}
		clrsetbits_le32(&mctl_com->cr, MCTL_CR_ROW_MASK,
				MCTL_CR_ROW(rows));
	} else {
		rows = (dram_para.para1 >> 16) & 0xff;
		writel(((dram_para.para2 & 0x000000f0) << 11) |
		       ((rows - 1) << 4) |
		       ((dram_para.para1 & 0x0f000000) >> 22) |
		       0x31000 | MCTL_CR_PAGE_SIZE(page_size),
		       &mctl_com->cr);
		setbits_le32(&mctl_com->swonr, 0x0003ffff);
	}

	/* Setup DRAM master priority? If this is left out things still work */
	writel(0x00000008, &mctl_com->mcr0_0);
	writel(0x0001000d, &mctl_com->mcr1_0);
	writel(0x00000004, &mctl_com->mcr0_1);
	writel(0x00000080, &mctl_com->mcr1_1);
	writel(0x00000004, &mctl_com->mcr0_2);
	writel(0x00000019, &mctl_com->mcr1_2);
	writel(0x00000004, &mctl_com->mcr0_3);
	writel(0x00000080, &mctl_com->mcr1_3);
	writel(0x00000004, &mctl_com->mcr0_4);
	writel(0x01010040, &mctl_com->mcr1_4);
	writel(0x00000004, &mctl_com->mcr0_5);
	writel(0x0001002f, &mctl_com->mcr1_5);
	writel(0x00000004, &mctl_com->mcr0_6);
	writel(0x00010020, &mctl_com->mcr1_6);
	writel(0x00000004, &mctl_com->mcr0_7);
	writel(0x00010020, &mctl_com->mcr1_7);
	writel(0x00000008, &mctl_com->mcr0_8);
	writel(0x00000001, &mctl_com->mcr1_8);
	writel(0x00000008, &mctl_com->mcr0_9);
	writel(0x00000005, &mctl_com->mcr1_9);
	writel(0x00000008, &mctl_com->mcr0_10);
	writel(0x00000003, &mctl_com->mcr1_10);
	writel(0x00000008, &mctl_com->mcr0_11);
	writel(0x00000005, &mctl_com->mcr1_11);
	writel(0x00000008, &mctl_com->mcr0_12);
	writel(0x00000003, &mctl_com->mcr1_12);
	writel(0x00000008, &mctl_com->mcr0_13);
	writel(0x00000004, &mctl_com->mcr1_13);
	writel(0x00000008, &mctl_com->mcr0_14);
	writel(0x00000002, &mctl_com->mcr1_14);
	writel(0x00000008, &mctl_com->mcr0_15);
	writel(0x00000003, &mctl_com->mcr1_15);
	writel(0x00010138, &mctl_com->bwcr);

	return 1 << (rows + columns + bus);
}
