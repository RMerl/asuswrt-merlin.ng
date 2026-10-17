// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2022 NXP
 */

#include <common.h>
#include <errno.h>
#include <log.h>
#include <asm/io.h>
#include <asm/arch/ddr.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#include <linux/delay.h>

static unsigned int g_cdd_rr_max[4];
static unsigned int g_cdd_rw_max[4];
static unsigned int g_cdd_wr_max[4];
static unsigned int g_cdd_ww_max[4];

#define MAX(a, b)	(((a) > (b)) ? (a) : (b))

void ddrphy_coldreset(void)
{
	/* dramphy_apb_n default 1 , assert -> 0, de_assert -> 1 */
	/* dramphy_reset_n default 0 , assert -> 0, de_assert -> 1 */
	/* dramphy_PwrOKIn default 0 , assert -> 1, de_assert -> 0 */

	/* src_gen_dphy_apb_sw_rst_de_assert */
	clrbits_le32(REG_SRC_DPHY_SW_CTRL, BIT(0));
	/* src_gen_dphy_sw_rst_de_assert */
	clrbits_le32(REG_SRC_DPHY_SINGLE_RESET_SW_CTRL, BIT(2));
	/* src_gen_dphy_PwrOKIn_sw_rst_de_assert() */
	setbits_le32(REG_SRC_DPHY_SINGLE_RESET_SW_CTRL, BIT(0));
	mdelay(10);

	/* src_gen_dphy_apb_sw_rst_assert */
	setbits_le32(REG_SRC_DPHY_SW_CTRL, BIT(0));
	/* src_gen_dphy_sw_rst_assert */
	setbits_le32(REG_SRC_DPHY_SINGLE_RESET_SW_CTRL, BIT(2));
	mdelay(10);
	/* src_gen_dphy_PwrOKIn_sw_rst_assert */
	clrbits_le32(REG_SRC_DPHY_SINGLE_RESET_SW_CTRL, BIT(0));
	mdelay(10);

	/* src_gen_dphy_apb_sw_rst_de_assert */
	clrbits_le32(REG_SRC_DPHY_SW_CTRL, BIT(0));
	/* src_gen_dphy_sw_rst_de_assert() */
	clrbits_le32(REG_SRC_DPHY_SINGLE_RESET_SW_CTRL, BIT(2));
}

void check_ddrc_idle(void)
{
	u32 regval;

	do {
		regval = readl(REG_DDRDSR_2);
		if (regval & BIT(31))
			break;
	} while (1);
}

void check_dfi_init_complete(void)
{
	u32 regval;

	do {
		regval = readl(REG_DDRDSR_2);
		if (regval & BIT(2))
			break;
	} while (1);
	setbits_le32(REG_DDRDSR_2, BIT(2));
}

void ddrc_config(struct dram_timing_info *dram_timing)
{
	u32 num = dram_timing->ddrc_cfg_num;
	struct dram_cfg_param *ddrc_config;
	int i = 0;

	ddrc_config = dram_timing->ddrc_cfg;
	for (i = 0; i < num; i++) {
		writel(ddrc_config->val, (ulong)ddrc_config->reg);
		ddrc_config++;
	}

	if (dram_timing->fsp_cfg) {
		ddrc_config = dram_timing->fsp_cfg[0].ddrc_cfg;
		while (ddrc_config->reg != 0) {
			writel(ddrc_config->val, (ulong)ddrc_config->reg);
			ddrc_config++;
		}
	}
}

static unsigned int look_for_max(unsigned int data[], unsigned int addr_start,
				 unsigned int addr_end)
{
	unsigned int i, imax = 0;

	for (i = addr_start; i <= addr_end; i++) {
		if (((data[i] >> 7) == 0) && data[i] > imax)
			imax = data[i];
	}

	return imax;
}

void get_trained_CDD(u32 fsp)
{
	unsigned int i, tmp;
	unsigned int cdd_cha[12], cdd_chb[12];
	unsigned int cdd_cha_rr_max, cdd_cha_rw_max, cdd_cha_wr_max, cdd_cha_ww_max;
	unsigned int cdd_chb_rr_max, cdd_chb_rw_max, cdd_chb_wr_max, cdd_chb_ww_max;

	for (i = 0; i < 6; i++) {
		tmp = dwc_ddrphy_apb_rd(0x54013 + i);
		cdd_cha[i * 2] = tmp & 0xff;
		cdd_cha[i * 2 + 1] = (tmp >> 8) & 0xff;
	}

	for (i = 0; i < 7; i++) {
		tmp = dwc_ddrphy_apb_rd(0x5402c + i);

		if (i == 0) {
			cdd_chb[0] = (tmp >> 8) & 0xff;
		} else if (i == 6) {
			cdd_chb[11] = tmp & 0xff;
		} else {
			cdd_chb[i * 2 - 1] = tmp & 0xff;
			cdd_chb[i * 2] = (tmp >> 8) & 0xff;
		}
	}

	cdd_cha_rr_max = look_for_max(cdd_cha, 0, 1);
	cdd_cha_rw_max = look_for_max(cdd_cha, 2, 5);
	cdd_cha_wr_max = look_for_max(cdd_cha, 6, 9);
	cdd_cha_ww_max = look_for_max(cdd_cha, 10, 11);
	cdd_chb_rr_max = look_for_max(cdd_chb, 0, 1);
	cdd_chb_rw_max = look_for_max(cdd_chb, 2, 5);
	cdd_chb_wr_max = look_for_max(cdd_chb, 6, 9);
	cdd_chb_ww_max = look_for_max(cdd_chb, 10, 11);
	g_cdd_rr_max[fsp] =  cdd_cha_rr_max > cdd_chb_rr_max ? cdd_cha_rr_max : cdd_chb_rr_max;
	g_cdd_rw_max[fsp] =  cdd_cha_rw_max > cdd_chb_rw_max ? cdd_cha_rw_max : cdd_chb_rw_max;
	g_cdd_wr_max[fsp] =  cdd_cha_wr_max > cdd_chb_wr_max ? cdd_cha_wr_max : cdd_chb_wr_max;
	g_cdd_ww_max[fsp] =  cdd_cha_ww_max > cdd_chb_ww_max ? cdd_cha_ww_max : cdd_chb_ww_max;
}

static u32 ddrc_get_fsp_reg_setting(struct dram_cfg_param *ddrc_cfg, unsigned int cfg_num, u32 reg)
{
	unsigned int i;

	for (i = 0; i < cfg_num; i++) {
		if (reg == ddrc_cfg[i].reg)
			return ddrc_cfg[i].val;
	}

	return 0;
}

static void ddrc_update_fsp_reg_setting(struct dram_cfg_param *ddrc_cfg, int cfg_num,
					u32 reg, u32 val)
{
	unsigned int i;

	for (i = 0; i < cfg_num; i++) {
		if (reg == ddrc_cfg[i].reg) {
			ddrc_cfg[i].val = val;
			return;
		}
	}
}

void update_umctl2_rank_space_setting(struct dram_timing_info *dram_timing, unsigned int pstat_num)
{
	u32 tmp, tmp_t;
	u32 wwt, rrt, wrt, rwt;
	u32 ext_wwt, ext_rrt, ext_wrt, ext_rwt;
	u32 max_wwt, max_rrt, max_wrt, max_rwt;
	u32 i;

	for (i = 0; i < pstat_num; i++) {
		/* read wwt, rrt, wrt, rwt fields from timing_cfg_0 */
		if (!dram_timing->fsp_cfg_num) {
			tmp = ddrc_get_fsp_reg_setting(dram_timing->ddrc_cfg,
						       dram_timing->ddrc_cfg_num,
						       REG_DDR_TIMING_CFG_0);
		} else {
			tmp = ddrc_get_fsp_reg_setting(dram_timing->fsp_cfg[i].ddrc_cfg,
						       ARRAY_SIZE(dram_timing->fsp_cfg[i].ddrc_cfg),
						       REG_DDR_TIMING_CFG_0);
		}
		wwt = (tmp >> 24) & 0x3;
		rrt = (tmp >> 26) & 0x3;
		wrt = (tmp >> 28) & 0x3;
		rwt = (tmp >> 30) & 0x3;

		/* read rxt_wwt, ext_rrt, ext_wrt, ext_rwt fields from timing_cfg_4 */
		if (!dram_timing->fsp_cfg_num) {
			tmp_t = ddrc_get_fsp_reg_setting(dram_timing->ddrc_cfg,
							 dram_timing->ddrc_cfg_num,
							 REG_DDR_TIMING_CFG_4);
		} else {
			tmp_t = ddrc_get_fsp_reg_setting(dram_timing->fsp_cfg[i].ddrc_cfg,
							 ARRAY_SIZE(dram_timing->fsp_cfg[i].ddrc_cfg),
							 REG_DDR_TIMING_CFG_4);
		}
		ext_wwt = (tmp_t >> 8)  & 0x3;
		ext_rrt = (tmp_t >> 10) & 0x3;
		ext_wrt = (tmp_t >> 12) & 0x3;
		ext_rwt = (tmp_t >> 14) & 0x3;

		wwt = (ext_wwt << 2) | wwt;
		rrt = (ext_rrt << 2) | rrt;
		wrt = (ext_wrt << 2) | wrt;
		rwt = (ext_rwt << 2) | rwt;

		max_wwt = MAX(g_cdd_ww_max[0], wwt);
		max_rrt = MAX(g_cdd_rr_max[0], rrt);
		max_wrt = MAX(g_cdd_wr_max[0], wrt);
		max_rwt = MAX(g_cdd_rw_max[0], rwt);
		/* verify values to see if are bigger then 15 (4 bits) */
		if (max_wwt > 15)
			max_wwt = 15;
		if (max_rrt > 15)
			max_rrt = 15;
		if (max_wrt > 15)
			max_wrt = 15;
		if (max_rwt > 15)
			max_rwt = 15;

		/* recalculate timings for controller registers */
		wwt = max_wwt & 0x3;
		rrt = max_rrt & 0x3;
		wrt = max_wrt & 0x3;
		rwt = max_rwt & 0x3;

		ext_wwt = (max_wwt & 0xC) >> 2;
		ext_rrt = (max_rrt & 0xC) >> 2;
		ext_wrt = (max_wrt & 0xC) >> 2;
		ext_rwt = (max_rwt & 0xC) >> 2;

		/* update timing_cfg_0 and timing_cfg_4 */
		tmp = (tmp & 0x00ffffff) | (rwt << 30) | (wrt << 28) |
			(rrt << 26) | (wwt << 24);
		tmp_t = (tmp_t & 0xFFFF00FF) | (ext_rwt << 14) |
			(ext_wrt << 12) | (ext_rrt << 10) | (ext_wwt << 8);

		if (!dram_timing->fsp_cfg_num) {
			ddrc_update_fsp_reg_setting(dram_timing->ddrc_cfg,
						    dram_timing->ddrc_cfg_num,
						    REG_DDR_TIMING_CFG_0, tmp);
			ddrc_update_fsp_reg_setting(dram_timing->ddrc_cfg,
						    dram_timing->ddrc_cfg_num,
						    REG_DDR_TIMING_CFG_4, tmp_t);
		} else {
			ddrc_update_fsp_reg_setting(dram_timing->fsp_cfg[i].ddrc_cfg,
						    ARRAY_SIZE(dram_timing->fsp_cfg[i].ddrc_cfg),
						    REG_DDR_TIMING_CFG_0, tmp);
			ddrc_update_fsp_reg_setting(dram_timing->fsp_cfg[i].ddrc_cfg,
						    ARRAY_SIZE(dram_timing->fsp_cfg[i].ddrc_cfg),
						    REG_DDR_TIMING_CFG_4, tmp_t);
		}
	}
}

u32 ddrc_mrr(u32 chip_select, u32 mode_reg_num, u32 *mode_reg_val)
{
	u32 temp;

	writel(0x80000000, REG_DDR_SDRAM_MD_CNTL_2);
	temp = 0x80000000 | (chip_select << 28) | (mode_reg_num << 0);
	writel(temp, REG_DDR_SDRAM_MD_CNTL);
	while ((readl(REG_DDR_SDRAM_MD_CNTL) & 0x80000000) == 0x80000000)
		;
	while (!(readl(REG_DDR_SDRAM_MPR5)))
		;
	*mode_reg_val = (readl(REG_DDR_SDRAM_MPR4) & 0xFF0000) >> 16;
	writel(0x0, REG_DDR_SDRAM_MPR5);
	while ((readl(REG_DDR_SDRAM_MPR5)))
		;
	writel(0x0, REG_DDR_SDRAM_MPR4);
	writel(0x0, REG_DDR_SDRAM_MD_CNTL_2);

	return 0;
}

void ddrc_mrs(u32 cs_sel, u32 opcode, u32 mr)
{
	u32 regval;

	regval = (cs_sel << 28) | (opcode << 6) | (mr);
	writel(regval, REG_DDR_SDRAM_MD_CNTL);
	setbits_le32(REG_DDR_SDRAM_MD_CNTL, BIT(31));
	check_ddrc_idle();
}

u32 lpddr4_mr_read(u32 mr_rank, u32 mr_addr)
{
	u32 chip_select, regval;

	if (mr_rank == 1)
		chip_select = 0; /* CS0 */
	else if (mr_rank == 2)
		chip_select = 1; /* CS1 */
	else
		chip_select = 4; /* CS0 & CS1 */

	ddrc_mrr(chip_select, mr_addr, &regval);

	return regval;
}

void update_mr_fsp_op0(struct dram_cfg_param *cfg, unsigned int num)
{
	int i;

	ddrc_mrs(0x4, 0x88, 13); /* FSP-OP->1, FSP-WR->0, VRCG=1, DMD=0 */
	for (i = 0; i < num; i++) {
		if (cfg[i].reg)
			ddrc_mrs(0x4, cfg[i].val, cfg[i].reg);
	}
	ddrc_mrs(0x4, 0xc0, 13); /* FSP-OP->1, FSP-WR->1, VRCG=0, DMD=0 */
}

void save_trained_mr12_14(struct dram_cfg_param *cfg, u32 cfg_num, u32 mr12, u32 mr14)
{
	int i;

	for (i = 0; i < cfg_num; i++) {
		if (cfg->reg == 12)
			cfg->val = mr12;
		else if (cfg->reg == 14)
			cfg->val = mr14;
		cfg++;
	}
}

int ddr_init(struct dram_timing_info *dram_timing)
{
	unsigned int initial_drate;
	struct dram_timing_info *saved_timing;
	void *fsp;
	int ret;
	u32 mr12, mr14;
	u32 regval;

	debug("DDRINFO: start DRAM init\n");

	/* reset ddrphy */
	ddrphy_coldreset();

	debug("DDRINFO: cfg clk\n");

	initial_drate = dram_timing->fsp_msg[0].drate;
	/* default to the frequency point 0 clock */
	ddrphy_init_set_dfi_clk(initial_drate);

	/*
	 * Start PHY initialization and training by
	 * accessing relevant PUB registers
	 */
	debug("DDRINFO:ddrphy config start\n");

	ret = ddr_cfg_phy(dram_timing);
	if (ret)
		return ret;

	debug("DDRINFO: ddrphy config done\n");

	update_umctl2_rank_space_setting(dram_timing, dram_timing->fsp_msg_num - 1);

	/* rogram the ddrc registers */
	debug("DDRINFO: ddrc config start\n");
	ddrc_config(dram_timing);
	debug("DDRINFO: ddrc config done\n");

#ifdef CONFIG_IMX9_DRAM_PM_COUNTER
	writel(0x200000, REG_DDR_DEBUG_19);
#endif

	check_dfi_init_complete();

	regval = readl(REG_DDR_SDRAM_CFG);
	writel((regval | 0x80000000), REG_DDR_SDRAM_CFG);

	check_ddrc_idle();

	mr12 = lpddr4_mr_read(1, 12);
	mr14 = lpddr4_mr_read(1, 14);

	/* save the dram timing config into memory */
	fsp = dram_config_save(dram_timing, CONFIG_SAVED_DRAM_TIMING_BASE);

	saved_timing = (struct dram_timing_info *)CONFIG_SAVED_DRAM_TIMING_BASE;
	saved_timing->fsp_cfg = fsp;
	saved_timing->fsp_cfg_num = dram_timing->fsp_cfg_num;
	if (saved_timing->fsp_cfg_num) {
		memcpy(saved_timing->fsp_cfg, dram_timing->fsp_cfg,
		       dram_timing->fsp_cfg_num * sizeof(struct dram_fsp_cfg));

		save_trained_mr12_14(saved_timing->fsp_cfg[0].mr_cfg,
				     ARRAY_SIZE(saved_timing->fsp_cfg[0].mr_cfg), mr12, mr14);
		/*
		 * Configure mode registers in fsp1 to mode register 0 because DDRC
		 * doesn't automatically set.
		 */
		if (saved_timing->fsp_cfg_num > 1)
			update_mr_fsp_op0(saved_timing->fsp_cfg[1].mr_cfg,
					  ARRAY_SIZE(saved_timing->fsp_cfg[1].mr_cfg));
	}

	return 0;
}

ulong ddrphy_addr_remap(u32 paddr_apb_from_ctlr)
{
	u32 paddr_apb_qual;
	u32 paddr_apb_unqual_dec_22_13;
	u32 paddr_apb_unqual_dec_19_13;
	u32 paddr_apb_unqual_dec_12_1;
	u32 paddr_apb_unqual;
	u32 paddr_apb_phy;

	paddr_apb_qual = (paddr_apb_from_ctlr << 1);
	paddr_apb_unqual_dec_22_13 = ((paddr_apb_qual & 0x7fe000) >> 13);
	paddr_apb_unqual_dec_12_1  = ((paddr_apb_qual & 0x1ffe) >> 1);

	switch (paddr_apb_unqual_dec_22_13) {
	case 0x000:
		paddr_apb_unqual_dec_19_13 = 0x00;
		break;
	case 0x001:
		paddr_apb_unqual_dec_19_13 = 0x01;
		break;
	case 0x002:
		paddr_apb_unqual_dec_19_13 = 0x02;
		break;
	case 0x003:
		paddr_apb_unqual_dec_19_13 = 0x03;
		break;
	case 0x004:
		paddr_apb_unqual_dec_19_13 = 0x04;
		break;
	case 0x005:
		paddr_apb_unqual_dec_19_13 = 0x05;
		break;
	case 0x006:
		paddr_apb_unqual_dec_19_13 = 0x06;
		break;
	case 0x007:
		paddr_apb_unqual_dec_19_13 = 0x07;
		break;
	case 0x008:
		paddr_apb_unqual_dec_19_13 = 0x08;
		break;
	case 0x009:
		paddr_apb_unqual_dec_19_13 = 0x09;
		break;
	case 0x00a:
		paddr_apb_unqual_dec_19_13 = 0x0a;
		break;
	case 0x00b:
		paddr_apb_unqual_dec_19_13 = 0x0b;
		break;
	case 0x100:
		paddr_apb_unqual_dec_19_13 = 0x0c;
		break;
	case 0x101:
		paddr_apb_unqual_dec_19_13 = 0x0d;
		break;
	case 0x102:
		paddr_apb_unqual_dec_19_13 = 0x0e;
		break;
	case 0x103:
		paddr_apb_unqual_dec_19_13 = 0x0f;
		break;
	case 0x104:
		paddr_apb_unqual_dec_19_13 = 0x10;
		break;
	case 0x105:
		paddr_apb_unqual_dec_19_13 = 0x11;
		break;
	case 0x106:
		paddr_apb_unqual_dec_19_13 = 0x12;
		break;
	case 0x107:
		paddr_apb_unqual_dec_19_13 = 0x13;
		break;
	case 0x108:
		paddr_apb_unqual_dec_19_13 = 0x14;
		break;
	case 0x109:
		paddr_apb_unqual_dec_19_13 = 0x15;
		break;
	case 0x10a:
		paddr_apb_unqual_dec_19_13 = 0x16;
		break;
	case 0x10b:
		paddr_apb_unqual_dec_19_13 = 0x17;
		break;
	case 0x200:
		paddr_apb_unqual_dec_19_13 = 0x18;
		break;
	case 0x201:
		paddr_apb_unqual_dec_19_13 = 0x19;
		break;
	case 0x202:
		paddr_apb_unqual_dec_19_13 = 0x1a;
		break;
	case 0x203:
		paddr_apb_unqual_dec_19_13 = 0x1b;
		break;
	case 0x204:
		paddr_apb_unqual_dec_19_13 = 0x1c;
		break;
	case 0x205:
		paddr_apb_unqual_dec_19_13 = 0x1d;
		break;
	case 0x206:
		paddr_apb_unqual_dec_19_13 = 0x1e;
		break;
	case 0x207:
		paddr_apb_unqual_dec_19_13 = 0x1f;
		break;
	case 0x208:
		paddr_apb_unqual_dec_19_13 = 0x20;
		break;
	case 0x209:
		paddr_apb_unqual_dec_19_13 = 0x21;
		break;
	case 0x20a:
		paddr_apb_unqual_dec_19_13 = 0x22;
		break;
	case 0x20b:
		paddr_apb_unqual_dec_19_13 = 0x23;
		break;
	case 0x300:
		paddr_apb_unqual_dec_19_13 = 0x24;
		break;
	case 0x301:
		paddr_apb_unqual_dec_19_13 = 0x25;
		break;
	case 0x302:
		paddr_apb_unqual_dec_19_13 = 0x26;
		break;
	case 0x303:
		paddr_apb_unqual_dec_19_13 = 0x27;
		break;
	case 0x304:
		paddr_apb_unqual_dec_19_13 = 0x28;
		break;
	case 0x305:
		paddr_apb_unqual_dec_19_13 = 0x29;
		break;
	case 0x306:
		paddr_apb_unqual_dec_19_13 = 0x2a;
		break;
	case 0x307:
		paddr_apb_unqual_dec_19_13 = 0x2b;
		break;
	case 0x308:
		paddr_apb_unqual_dec_19_13 = 0x2c;
		break;
	case 0x309:
		paddr_apb_unqual_dec_19_13 = 0x2d;
		break;
	case 0x30a:
		paddr_apb_unqual_dec_19_13 = 0x2e;
		break;
	case 0x30b:
		paddr_apb_unqual_dec_19_13 = 0x2f;
		break;
	case 0x010:
		paddr_apb_unqual_dec_19_13 = 0x30;
		break;
	case 0x011:
		paddr_apb_unqual_dec_19_13 = 0x31;
		break;
	case 0x012:
		paddr_apb_unqual_dec_19_13 = 0x32;
		break;
	case 0x013:
		paddr_apb_unqual_dec_19_13 = 0x33;
		break;
	case 0x014:
		paddr_apb_unqual_dec_19_13 = 0x34;
		break;
	case 0x015:
		paddr_apb_unqual_dec_19_13 = 0x35;
		break;
	case 0x016:
		paddr_apb_unqual_dec_19_13 = 0x36;
		break;
	case 0x017:
		paddr_apb_unqual_dec_19_13 = 0x37;
		break;
	case 0x018:
		paddr_apb_unqual_dec_19_13 = 0x38;
		break;
	case 0x019:
		paddr_apb_unqual_dec_19_13 = 0x39;
		break;
	case 0x110:
		paddr_apb_unqual_dec_19_13 = 0x3a;
		break;
	case 0x111:
		paddr_apb_unqual_dec_19_13 = 0x3b;
		break;
	case 0x112:
		paddr_apb_unqual_dec_19_13 = 0x3c;
		break;
	case 0x113:
		paddr_apb_unqual_dec_19_13 = 0x3d;
		break;
	case 0x114:
		paddr_apb_unqual_dec_19_13 = 0x3e;
		break;
	case 0x115:
		paddr_apb_unqual_dec_19_13 = 0x3f;
		break;
	case 0x116:
		paddr_apb_unqual_dec_19_13 = 0x40;
		break;
	case 0x117:
		paddr_apb_unqual_dec_19_13 = 0x41;
		break;
	case 0x118:
		paddr_apb_unqual_dec_19_13 = 0x42;
		break;
	case 0x119:
		paddr_apb_unqual_dec_19_13 = 0x43;
		break;
	case 0x210:
		paddr_apb_unqual_dec_19_13 = 0x44;
		break;
	case 0x211:
		paddr_apb_unqual_dec_19_13 = 0x45;
		break;
	case 0x212:
		paddr_apb_unqual_dec_19_13 = 0x46;
		break;
	case 0x213:
		paddr_apb_unqual_dec_19_13 = 0x47;
		break;
	case 0x214:
		paddr_apb_unqual_dec_19_13 = 0x48;
		break;
	case 0x215:
		paddr_apb_unqual_dec_19_13 = 0x49;
		break;
	case 0x216:
		paddr_apb_unqual_dec_19_13 = 0x4a;
		break;
	case 0x217:
		paddr_apb_unqual_dec_19_13 = 0x4b;
		break;
	case 0x218:
		paddr_apb_unqual_dec_19_13 = 0x4c;
		break;
	case 0x219:
		paddr_apb_unqual_dec_19_13 = 0x4d;
		break;
	case 0x310:
		paddr_apb_unqual_dec_19_13 = 0x4e;
		break;
	case 0x311:
		paddr_apb_unqual_dec_19_13 = 0x4f;
		break;
	case 0x312:
		paddr_apb_unqual_dec_19_13 = 0x50;
		break;
	case 0x313:
		paddr_apb_unqual_dec_19_13 = 0x51;
		break;
	case 0x314:
		paddr_apb_unqual_dec_19_13 = 0x52;
		break;
	case 0x315:
		paddr_apb_unqual_dec_19_13 = 0x53;
		break;
	case 0x316:
		paddr_apb_unqual_dec_19_13 = 0x54;
		break;
	case 0x317:
		paddr_apb_unqual_dec_19_13 = 0x55;
		break;
	case 0x318:
		paddr_apb_unqual_dec_19_13 = 0x56;
		break;
	case 0x319:
		paddr_apb_unqual_dec_19_13 = 0x57;
		break;
	case 0x020:
		paddr_apb_unqual_dec_19_13 = 0x58;
		break;
	case 0x120:
		paddr_apb_unqual_dec_19_13 = 0x59;
		break;
	case 0x220:
		paddr_apb_unqual_dec_19_13 = 0x5a;
		break;
	case 0x320:
		paddr_apb_unqual_dec_19_13 = 0x5b;
		break;
	case 0x040:
		paddr_apb_unqual_dec_19_13 = 0x5c;
		break;
	case 0x140:
		paddr_apb_unqual_dec_19_13 = 0x5d;
		break;
	case 0x240:
		paddr_apb_unqual_dec_19_13 = 0x5e;
		break;
	case 0x340:
		paddr_apb_unqual_dec_19_13 = 0x5f;
		break;
	case 0x050:
		paddr_apb_unqual_dec_19_13 = 0x60;
		break;
	case 0x051:
		paddr_apb_unqual_dec_19_13 = 0x61;
		break;
	case 0x052:
		paddr_apb_unqual_dec_19_13 = 0x62;
		break;
	case 0x053:
		paddr_apb_unqual_dec_19_13 = 0x63;
		break;
	case 0x054:
		paddr_apb_unqual_dec_19_13 = 0x64;
		break;
	case 0x055:
		paddr_apb_unqual_dec_19_13 = 0x65;
		break;
	case 0x056:
		paddr_apb_unqual_dec_19_13 = 0x66;
		break;
	case 0x057:
		paddr_apb_unqual_dec_19_13 = 0x67;
		break;
	case 0x070:
		paddr_apb_unqual_dec_19_13 = 0x68;
		break;
	case 0x090:
		paddr_apb_unqual_dec_19_13 = 0x69;
		break;
	case 0x190:
		paddr_apb_unqual_dec_19_13 = 0x6a;
		break;
	case 0x290:
		paddr_apb_unqual_dec_19_13 = 0x6b;
		break;
	case 0x390:
		paddr_apb_unqual_dec_19_13 = 0x6c;
		break;
	case 0x0c0:
		paddr_apb_unqual_dec_19_13 = 0x6d;
		break;
	case 0x0d0:
		paddr_apb_unqual_dec_19_13 = 0x6e;
		break;
	default:
		paddr_apb_unqual_dec_19_13 = 0x00;
		break;
	}

	paddr_apb_unqual = ((paddr_apb_unqual_dec_19_13 << 13) | (paddr_apb_unqual_dec_12_1 << 1));

	paddr_apb_phy = (paddr_apb_unqual << 1);

	return paddr_apb_phy;
}
