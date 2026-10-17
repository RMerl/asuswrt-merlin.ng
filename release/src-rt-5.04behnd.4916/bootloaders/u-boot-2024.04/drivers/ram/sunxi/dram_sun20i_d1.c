// SPDX-License-Identifier: GPL-2.0+
/*
 * Allwinner D1/D1s/R528/T113-sx DRAM initialisation
 *
 * As usual there is no documentation for the memory controller or PHY IP
 * used here. The baseline of this code was lifted from awboot[1], which
 * seems to be based on some form of de-compilation of some original Allwinner
 * code bits (with a GPL2 license tag from the very beginning).
 * This version here is a reworked version, to match the U-Boot coding style
 * and style of the other Allwinner DRAM drivers.
 *
 * [1] https://github.com/szemzoa/awboot.git
 */

#include <asm/io.h>
#include <common.h>
#ifdef CONFIG_RAM
  #include <dm.h>
  #include <ram.h>
#endif
#include <linux/delay.h>

#include "dram_sun20i_d1.h"

#ifndef SUNXI_SID_BASE
#define SUNXI_SID_BASE	0x3006200
#endif

#ifndef SUNXI_CCM_BASE
#define SUNXI_CCM_BASE	0x2001000
#endif

static void sid_read_ldoB_cal(const dram_para_t *para)
{
	uint32_t reg;

	reg = (readl(SUNXI_SID_BASE + 0x1c) & 0xff00) >> 8;

	if (reg == 0)
		return;

	switch (para->dram_type) {
	case SUNXI_DRAM_TYPE_DDR2:
		break;
	case SUNXI_DRAM_TYPE_DDR3:
		if (reg > 0x20)
			reg -= 0x16;
		break;
	default:
		reg = 0;
		break;
	}

	clrsetbits_le32(0x3000150, 0xff00, reg << 8);
}

static void dram_voltage_set(const dram_para_t *para)
{
	int vol;

	switch (para->dram_type) {
	case SUNXI_DRAM_TYPE_DDR2:
		vol = 47;
		break;
	case SUNXI_DRAM_TYPE_DDR3:
		vol = 25;
		break;
	default:
		vol = 0;
		break;
	}

	clrsetbits_le32(0x3000150, 0x20ff00, vol << 8);

	udelay(1);

	sid_read_ldoB_cal(para);
}

static void dram_enable_all_master(void)
{
	writel(~0, 0x3102020);
	writel(0xff, 0x3102024);
	writel(0xffff, 0x3102028);
	udelay(10);
}

static void dram_disable_all_master(void)
{
	writel(1, 0x3102020);
	writel(0, 0x3102024);
	writel(0, 0x3102028);
	udelay(10);
}

static void eye_delay_compensation(const dram_para_t *para)
{
	uint32_t delay;
	unsigned long ptr;

	// DATn0IOCR, n =  0...7
	delay = (para->dram_tpr11 & 0xf) << 9;
	delay |= (para->dram_tpr12 & 0xf) << 1;
	for (ptr = 0x3103310; ptr < 0x3103334; ptr += 4)
		setbits_le32(ptr, delay);

	// DATn1IOCR, n =  0...7
	delay = (para->dram_tpr11 & 0xf0) << 5;
	delay |= (para->dram_tpr12 & 0xf0) >> 3;
	for (ptr = 0x3103390; ptr != 0x31033b4; ptr += 4)
		setbits_le32(ptr, delay);

	// PGCR0: assert AC loopback FIFO reset
	clrbits_le32(0x3103100, 0x04000000);

	// ??

	delay = (para->dram_tpr11 & 0xf0000) >> 7;
	delay |= (para->dram_tpr12 & 0xf0000) >> 15;
	setbits_le32(0x3103334, delay);
	setbits_le32(0x3103338, delay);

	delay = (para->dram_tpr11 & 0xf00000) >> 11;
	delay |= (para->dram_tpr12 & 0xf00000) >> 19;
	setbits_le32(0x31033b4, delay);
	setbits_le32(0x31033b8, delay);

	setbits_le32(0x310333c, (para->dram_tpr11 & 0xf0000) << 9);
	setbits_le32(0x31033bc, (para->dram_tpr11 & 0xf00000) << 5);

	// PGCR0: release AC loopback FIFO reset
	setbits_le32(0x3103100, BIT(26));

	udelay(1);

	delay = (para->dram_tpr10 & 0xf0) << 4;
	for (ptr = 0x3103240; ptr != 0x310327c; ptr += 4)
		setbits_le32(ptr, delay);
	for (ptr = 0x3103228; ptr != 0x3103240; ptr += 4)
		setbits_le32(ptr, delay);

	setbits_le32(0x3103218, (para->dram_tpr10 & 0x0f) << 8);
	setbits_le32(0x310321c, (para->dram_tpr10 & 0x0f) << 8);

	setbits_le32(0x3103280, (para->dram_tpr10 & 0xf00) >> 4);
}

/*
 * Main purpose of the auto_set_timing routine seems to be to calculate all
 * timing settings for the specific type of sdram used. Read together with
 * an sdram datasheet for context on the various variables.
 */
static void mctl_set_timing_params(const dram_para_t *para,
				   const dram_config_t *config)
{
	/* DRAM_TPR0 */
	u8 tccd		= 2;
	u8 tfaw;
	u8 trrd;
	u8 trcd;
	u8 trc;

	/* DRAM_TPR1 */
	u8 txp;
	u8 twtr;
	u8 trtp		= 4;
	u8 twr;
	u8 trp;
	u8 tras;

	/* DRAM_TPR2 */
	u16 trefi;
	u16 trfc;

	u8 tcksrx;
	u8 tckesr;
	u8 trd2wr;
	u8 twr2rd;
	u8 trasmax;
	u8 twtp;
	u8 tcke;
	u8 tmod;
	u8 tmrd;
	u8 tmrw;

	u8 tcl;
	u8 tcwl;
	u8 t_rdata_en;
	u8 wr_latency;

	u32 mr0;
	u32 mr1;
	u32 mr2;
	u32 mr3;

	u32 tdinit0;
	u32 tdinit1;
	u32 tdinit2;
	u32 tdinit3;

	switch (para->dram_type) {
	case SUNXI_DRAM_TYPE_DDR2:
		/* DRAM_TPR0 */
		tfaw		= ns_to_t(50);
		trrd		= ns_to_t(10);
		trcd		= ns_to_t(20);
		trc		= ns_to_t(65);

		/* DRAM_TPR1 */
		txp		= 2;
		twtr		= ns_to_t(8);
		twr		= ns_to_t(15);
		trp		= ns_to_t(15);
		tras		= ns_to_t(45);

		/* DRAM_TRP2 */
		trfc		= ns_to_t(328);
		trefi		= ns_to_t(7800) / 32;

		trasmax		= CONFIG_DRAM_CLK / 30;
		if (CONFIG_DRAM_CLK < 409) {
			t_rdata_en	= 1;
			tcl		= 3;
			mr0		= 0x06a3;
		} else {
			t_rdata_en	= 2;
			tcl		= 4;
			mr0		= 0x0e73;
		}
		tmrd		= 2;
		twtp		= twr + 5;
		tcksrx		= 5;
		tckesr		= 4;
		trd2wr		= 4;
		tcke		= 3;
		tmod		= 12;
		wr_latency	= 1;
		tmrw		= 0;
		twr2rd		= twtr + 5;
		tcwl		= 0;

		mr1		= para->dram_mr1;
		mr2		= 0;
		mr3		= 0;

		tdinit0		= 200 * CONFIG_DRAM_CLK + 1;
		tdinit1		= 100 * CONFIG_DRAM_CLK / 1000 + 1;
		tdinit2		= 200 * CONFIG_DRAM_CLK + 1;
		tdinit3		= 1 * CONFIG_DRAM_CLK + 1;

		break;
	case SUNXI_DRAM_TYPE_DDR3:
		trfc		= ns_to_t(350);
		trefi		= ns_to_t(7800) / 32 + 1;	// XXX

		twtr		= ns_to_t(8) + 2;		// + 2 ? XXX
		/* Only used by trd2wr calculation, which gets discard below */
//		twr		= max(ns_to_t(15), 2);
		trrd		= max(ns_to_t(10), 2);
		txp		= max(ns_to_t(10), 2);

		if (CONFIG_DRAM_CLK <= 800) {
			tfaw		= ns_to_t(50);
			trcd		= ns_to_t(15);
			trp		= ns_to_t(15);
			trc		= ns_to_t(53);
			tras		= ns_to_t(38);

			mr0		= 0x1c70;
			mr2		= 0x18;
			tcl		= 6;
			wr_latency	= 2;
			tcwl		= 4;
			t_rdata_en	= 4;
		} else {
			tfaw		= ns_to_t(35);
			trcd		= ns_to_t(14);
			trp		= ns_to_t(14);
			trc		= ns_to_t(48);
			tras		= ns_to_t(34);

			mr0		= 0x1e14;
			mr2		= 0x20;
			tcl		= 7;
			wr_latency	= 3;
			tcwl		= 5;
			t_rdata_en	= 5;
		}

		trasmax		= CONFIG_DRAM_CLK / 30;
		twtp		= tcwl + 2 + twtr;		// WL+BL/2+tWTR
		/* Gets overwritten below */
//		trd2wr		= tcwl + 2 + twr;		// WL+BL/2+tWR
		twr2rd		= tcwl + twtr;			// WL+tWTR

		tdinit0		= 500 * CONFIG_DRAM_CLK + 1;	// 500 us
		tdinit1		= 360 * CONFIG_DRAM_CLK / 1000 + 1;   // 360 ns
		tdinit2		= 200 * CONFIG_DRAM_CLK + 1;	// 200 us
		tdinit3		= 1 * CONFIG_DRAM_CLK + 1;	//   1 us

		mr1		= para->dram_mr1;
		mr3		= 0;
		tcke		= 3;
		tcksrx		= 5;
		tckesr		= 4;
		if (((config->dram_tpr13 & 0xc) == 0x04) || CONFIG_DRAM_CLK < 912)
			trd2wr	   = 5;
		else
			trd2wr	   = 6;

		tmod		= 12;
		tmrd		= 4;
		tmrw		= 0;

		break;
	case SUNXI_DRAM_TYPE_LPDDR2:
		tfaw		= max(ns_to_t(50), 4);
		trrd		= max(ns_to_t(10), 1);
		trcd		= max(ns_to_t(24), 2);
		trc		= ns_to_t(70);
		txp		= ns_to_t(8);
		if (txp < 2) {
			txp++;
			twtr	= 2;
		} else {
			twtr	= txp;
		}
		twr		= max(ns_to_t(15), 2);
		trp		= ns_to_t(17);
		tras		= ns_to_t(42);
		trefi		= ns_to_t(3900) / 32;
		trfc		= ns_to_t(210);

		trasmax		= CONFIG_DRAM_CLK / 60;
		mr3		= para->dram_mr3;
		twtp		= twr + 5;
		mr2		= 6;
		mr1		= 5;
		tcksrx		= 5;
		tckesr		= 5;
		trd2wr		= 10;
		tcke		= 2;
		tmod		= 5;
		tmrd		= 5;
		tmrw		= 3;
		tcl		= 4;
		wr_latency	= 1;
		t_rdata_en	= 1;

		tdinit0		= 200 * CONFIG_DRAM_CLK + 1;
		tdinit1		= 100 * CONFIG_DRAM_CLK / 1000 + 1;
		tdinit2		= 11 * CONFIG_DRAM_CLK + 1;
		tdinit3		= 1 * CONFIG_DRAM_CLK + 1;
		twr2rd		= twtr + 5;
		tcwl		= 2;
		mr1		= 195;
		mr0		= 0;

		break;
	case SUNXI_DRAM_TYPE_LPDDR3:
		tfaw		= max(ns_to_t(50), 4);
		trrd		= max(ns_to_t(10), 1);
		trcd		= max(ns_to_t(24), 2);
		trc		= ns_to_t(70);
		twtr		= max(ns_to_t(8), 2);
		twr		= max(ns_to_t(15), 2);
		trp		= ns_to_t(17);
		tras		= ns_to_t(42);
		trefi		= ns_to_t(3900) / 32;
		trfc		= ns_to_t(210);
		txp		= twtr;

		trasmax		= CONFIG_DRAM_CLK / 60;
		if (CONFIG_DRAM_CLK < 800) {
			tcwl	   = 4;
			wr_latency = 3;
			t_rdata_en = 6;
			mr2		   = 12;
		} else {
			tcwl	   = 3;
			tcke	   = 6;
			wr_latency = 2;
			t_rdata_en = 5;
			mr2		   = 10;
		}
		twtp		= tcwl + 5;
		tcl		= 7;
		mr3		= para->dram_mr3;
		tcksrx		= 5;
		tckesr		= 5;
		trd2wr		= 13;
		tcke		= 3;
		tmod		= 12;
		tdinit0		= 400 * CONFIG_DRAM_CLK + 1;
		tdinit1		= 500 * CONFIG_DRAM_CLK / 1000 + 1;
		tdinit2		= 11 * CONFIG_DRAM_CLK + 1;
		tdinit3		= 1 * CONFIG_DRAM_CLK + 1;
		tmrd		= 5;
		tmrw		= 5;
		twr2rd		= tcwl + twtr + 5;
		mr1		= 195;
		mr0		= 0;

		break;
	default:
		trfc		= 128;
		trp		= 6;
		trefi		= 98;
		txp		= 10;
		twr		= 8;
		twtr		= 3;
		tras		= 14;
		tfaw		= 16;
		trc		= 20;
		trcd		= 6;
		trrd		= 3;

		twr2rd		= 8;
		tcksrx		= 4;
		tckesr		= 3;
		trd2wr		= 4;
		trasmax		= 27;
		twtp		= 12;
		tcke		= 2;
		tmod		= 6;
		tmrd		= 2;
		tmrw		= 0;
		tcwl		= 3;
		tcl		= 3;
		wr_latency	= 1;
		t_rdata_en	= 1;
		mr3		= 0;
		mr2		= 0;
		mr1		= 0;
		mr0		= 0;
		tdinit3		= 0;
		tdinit2		= 0;
		tdinit1		= 0;
		tdinit0		= 0;

		break;
	}

	/* Set mode registers */
	writel(mr0, 0x3103030);
	writel(mr1, 0x3103034);
	writel(mr2, 0x3103038);
	writel(mr3, 0x310303c);
	/* TODO: dram_odt_en is either 0x0 or 0x1, so right shift looks weird */
	writel((para->dram_odt_en >> 4) & 0x3, 0x310302c);

	/* Set dram timing DRAMTMG0 - DRAMTMG5 */
	writel((twtp << 24) | (tfaw << 16) | (trasmax << 8) | (tras << 0),
		0x3103058);
	writel((txp << 16) | (trtp << 8) | (trc << 0),
		0x310305c);
	writel((tcwl << 24) | (tcl << 16) | (trd2wr << 8) | (twr2rd << 0),
		0x3103060);
	writel((tmrw << 16) | (tmrd << 12) | (tmod << 0),
		0x3103064);
	writel((trcd << 24) | (tccd << 16) | (trrd << 8) | (trp << 0),
		0x3103068);
	writel((tcksrx << 24) | (tcksrx << 16) | (tckesr << 8) | (tcke << 0),
		0x310306c);

	/* Set dual rank timing */
	clrsetbits_le32(0x3103078, 0xf000ffff,
			(CONFIG_DRAM_CLK < 800) ? 0xf0006610 : 0xf0007610);

	/* Set phy interface time PITMG0, PTR3, PTR4 */
	writel((0x2 << 24) | (t_rdata_en << 16) | BIT(8) | (wr_latency << 0),
		0x3103080);
	writel(((tdinit0 << 0) | (tdinit1 << 20)), 0x3103050);
	writel(((tdinit2 << 0) | (tdinit3 << 20)), 0x3103054);

	/* Set refresh timing and mode */
	writel((trefi << 16) | (trfc << 0), 0x3103090);
	writel((trefi << 15) & 0x0fff0000, 0x3103094);
}

// Purpose of this routine seems to be to initialize the PLL driving
// the MBUS and sdram.
//
static int ccu_set_pll_ddr_clk(int index, const dram_para_t *para,
			       const dram_config_t *config)
{
	unsigned int val, clk, n;

	if (config->dram_tpr13 & BIT(6))
		clk = para->dram_tpr9;
	else
		clk = para->dram_clk;

	// set VCO clock divider
	n = (clk * 2) / 24;

	val = readl(SUNXI_CCM_BASE + 0x10);
	val &= ~0x0007ff03;			// clear dividers
	val |= (n - 1) << 8;			// set PLL division
	val |= BIT(31) | BIT(30);		// enable PLL and LDO
	writel(val | BIT(29), SUNXI_CCM_BASE + 0x10);

	// wait for PLL to lock
	while ((readl(SUNXI_CCM_BASE + 0x10) & BIT(28)) == 0)
		;

	udelay(20);

	// enable PLL output
	setbits_le32(SUNXI_CCM_BASE + 0x0, BIT(27));

	// turn clock gate on
	val = readl(SUNXI_CCM_BASE + 0x800);
	val &= ~0x03000303;		// select DDR clk source, n=1, m=1
	val |= BIT(31);			// turn clock on
	writel(val, SUNXI_CCM_BASE + 0x800);

	return n * 24;
}

/* Set up the PLL and clock gates for the DRAM controller and MBUS clocks. */
static void mctl_sys_init(const dram_para_t *para, const dram_config_t *config)
{
	// assert MBUS reset
	clrbits_le32(SUNXI_CCM_BASE + 0x540, BIT(30));

	// turn off sdram clock gate, assert sdram reset
	clrbits_le32(SUNXI_CCM_BASE + 0x80c, 0x10001);
	clrsetbits_le32(SUNXI_CCM_BASE + 0x800, BIT(31) | BIT(30), BIT(27));
	udelay(10);

	// set ddr pll clock
	ccu_set_pll_ddr_clk(0, para, config);
	udelay(100);
	dram_disable_all_master();

	// release sdram reset
	setbits_le32(SUNXI_CCM_BASE + 0x80c, BIT(16));

	// release MBUS reset
	setbits_le32(SUNXI_CCM_BASE + 0x540, BIT(30));
	setbits_le32(SUNXI_CCM_BASE + 0x800, BIT(30));

	udelay(5);

	// turn on sdram clock gate
	setbits_le32(SUNXI_CCM_BASE + 0x80c, BIT(0));

	// turn dram clock gate on, trigger sdr clock update
	setbits_le32(SUNXI_CCM_BASE + 0x800, BIT(31) | BIT(27));
	udelay(5);

	// mCTL clock enable
	writel(0x8000, 0x310300c);
	udelay(10);
}

// The main purpose of this routine seems to be to copy an address configuration
// from the dram_para1 and dram_para2 fields to the PHY configuration registers
// (0x3102000, 0x3102004).
//
static void mctl_com_init(const dram_para_t *para, const dram_config_t *config)
{
	uint32_t val, width;
	unsigned long ptr;
	int i;

	// purpose ??
	clrsetbits_le32(0x3102008, 0x3f00, 0x2000);

	// set SDRAM type and word width
	val  = readl(0x3102000) & ~0x00fff000;
	val |= (para->dram_type & 0x7) << 16;		// DRAM type
	val |= (~config->dram_para2 & 0x1) << 12;		// DQ width
	val |= BIT(22);					// ??
	if (para->dram_type == SUNXI_DRAM_TYPE_LPDDR2 ||
	    para->dram_type == SUNXI_DRAM_TYPE_LPDDR3) {
		val |= BIT(19);		// type 6 and 7 must use 1T
	} else {
		if (config->dram_tpr13 & BIT(5))
			val |= BIT(19);
	}
	writel(val, 0x3102000);

	// init rank / bank / row for single/dual or two different ranks
	if ((config->dram_para2 & BIT(8)) &&
	    ((config->dram_para2 & 0xf000) != 0x1000))
		width = 32;
	else
		width = 16;

	ptr = 0x3102000;
	for (i = 0; i < width; i += 16) {
		val = readl(ptr) & 0xfffff000;

		val |= (config->dram_para2 >> 12) & 0x3; // rank
		val |= ((config->dram_para1 >> (i + 12)) << 2) & 0x4; // bank - 2
		val |= (((config->dram_para1 >> (i + 4)) - 1) << 4) & 0xff; // row - 1

		// convert from page size to column addr width - 3
		switch ((config->dram_para1 >> i) & 0xf) {
		case 8: val |= 0xa00; break;
		case 4: val |= 0x900; break;
		case 2: val |= 0x800; break;
		case 1: val |= 0x700; break;
		default: val |= 0x600; break;
		}
		writel(val, ptr);
		ptr += 4;
	}

	// set ODTMAP based on number of ranks in use
	val = (readl(0x3102000) & 0x1) ? 0x303 : 0x201;
	writel(val, 0x3103120);

	// set mctl reg 3c4 to zero when using half DQ
	if (config->dram_para2 & BIT(0))
		writel(0, 0x31033c4);

	// purpose ??
	if (para->dram_tpr4) {
                setbits_le32(0x3102000, (para->dram_tpr4 & 0x3) << 25);
                setbits_le32(0x3102004, (para->dram_tpr4 & 0x7fc) << 10);
	}
}

static const uint8_t ac_remapping_tables[][22] = {
	[0] = { 0 },
	[1] = {  1,  9,  3,  7,  8, 18,  4, 13,  5,  6, 10,
		 2, 14, 12,  0,  0, 21, 17, 20, 19, 11, 22 },
	[2] = {  4,  9,  3,  7,  8, 18,  1, 13,  2,  6, 10,
		 5, 14, 12,  0,  0, 21, 17, 20, 19, 11, 22 },
	[3] = {  1,  7,  8, 12, 10, 18,  4, 13,  5,  6,  3,
		 2,  9,  0,  0,  0, 21, 17, 20, 19, 11, 22 },
	[4] = {  4, 12, 10,  7,  8, 18,  1, 13,  2,  6,  3,
		 5,  9,  0,  0,  0, 21, 17, 20, 19, 11, 22 },
	[5] = { 13,  2,  7,  9, 12, 19,  5,  1,  6,  3,  4,
		 8, 10,  0,  0,  0, 21, 22, 18, 17, 11, 20 },
	[6] = {  3, 10,  7, 13,  9, 11,  1,  2,  4,  6,  8,
		 5, 12,  0,  0,  0, 20,  1,  0, 21, 22, 17 },
	[7] = {  3,  2,  4,  7,  9,  1, 17, 12, 18, 14, 13,
		 8, 15,  6, 10,  5, 19, 22, 16, 21, 20, 11 },
};

/*
 * This routine chooses one of several remapping tables for 22 lines.
 * It is unclear which lines are being remapped. It seems to pick
 * table cfg7 for the Nezha board.
 */
static void mctl_phy_ac_remapping(const dram_para_t *para,
				  const dram_config_t *config)
{
	const uint8_t *cfg;
	uint32_t fuse, val;

	/*
	 * It is unclear whether the LPDDRx types don't need any remapping,
	 * or whether the original code just didn't provide tables.
	 */
	if (para->dram_type != SUNXI_DRAM_TYPE_DDR2 &&
	    para->dram_type != SUNXI_DRAM_TYPE_DDR3)
		return;

	fuse = (readl(SUNXI_SID_BASE + 0x28) & 0xf00) >> 8;
	debug("DDR efuse: 0x%x\n", fuse);

	if (para->dram_type == SUNXI_DRAM_TYPE_DDR2) {
		if (fuse == 15)
			return;
		cfg = ac_remapping_tables[6];
	} else {
		if (config->dram_tpr13 & 0xc0000) {
			cfg = ac_remapping_tables[7];
		} else {
			switch (fuse) {
			case 8: cfg = ac_remapping_tables[2]; break;
			case 9: cfg = ac_remapping_tables[3]; break;
			case 10: cfg = ac_remapping_tables[5]; break;
			case 11: cfg = ac_remapping_tables[4]; break;
			default:
			case 12: cfg = ac_remapping_tables[1]; break;
			case 13:
			case 14: cfg = ac_remapping_tables[0]; break;
			}
		}
	}

	val = (cfg[4] << 25) | (cfg[3] << 20) | (cfg[2] << 15) |
	      (cfg[1] << 10) | (cfg[0] << 5);
	writel(val, 0x3102500);

	val = (cfg[10] << 25) | (cfg[9] << 20) | (cfg[8] << 15) |
	      (cfg[ 7] << 10) | (cfg[6] <<  5) | cfg[5];
	writel(val, 0x3102504);

	val = (cfg[15] << 20) | (cfg[14] << 15) | (cfg[13] << 10) |
	      (cfg[12] <<  5) | cfg[11];
	writel(val, 0x3102508);

	val = (cfg[21] << 25) | (cfg[20] << 20) | (cfg[19] << 15) |
	      (cfg[18] << 10) | (cfg[17] <<  5) | cfg[16];
	writel(val, 0x310250c);

	val = (cfg[4] << 25) | (cfg[3] << 20) | (cfg[2] << 15) |
	      (cfg[1] << 10) | (cfg[0] <<  5) | 1;
	writel(val, 0x3102500);
}

// Init the controller channel. The key part is placing commands in the main
// command register (PIR, 0x3103000) and checking command status (PGSR0, 0x3103010).
//
static unsigned int mctl_channel_init(unsigned int ch_index,
				      const dram_para_t *para,
				      const dram_config_t *config)
{
	unsigned int val, dqs_gating_mode;

	dqs_gating_mode = (config->dram_tpr13 & 0xc) >> 2;

	// set DDR clock to half of CPU clock
	clrsetbits_le32(0x310200c, 0xfff, (para->dram_clk / 2) - 1);

	// MRCTRL0 nibble 3 undocumented
	clrsetbits_le32(0x3103108, 0xf00, 0x300);

	if (para->dram_odt_en)
		val = 0;
	else
		val = BIT(5);

	// DX0GCR0
	if (para->dram_clk > 672)
		clrsetbits_le32(0x3103344, 0xf63e, val);
	else
		clrsetbits_le32(0x3103344, 0xf03e, val);

	// DX1GCR0
	if (para->dram_clk > 672) {
                setbits_le32(0x3103344, 0x400);
		clrsetbits_le32(0x31033c4, 0xf63e, val);
	} else {
		clrsetbits_le32(0x31033c4, 0xf03e, val);
	}

	// 0x3103208 undocumented
	setbits_le32(0x3103208, BIT(1));

	eye_delay_compensation(para);

	// set PLL SSCG ?
	val = readl(0x3103108);
	if (dqs_gating_mode == 1) {
		clrsetbits_le32(0x3103108, 0xc0, 0);
		clrbits_le32(0x31030bc, 0x107);
	} else if (dqs_gating_mode == 2) {
		clrsetbits_le32(0x3103108, 0xc0, 0x80);

		clrsetbits_le32(0x31030bc, 0x107,
				(((config->dram_tpr13 >> 16) & 0x1f) - 2) | 0x100);
		clrsetbits_le32(0x310311c, BIT(31), BIT(27));
	} else {
		clrbits_le32(0x3103108, 0x40);
		udelay(10);
		setbits_le32(0x3103108, 0xc0);
	}

	if (para->dram_type == SUNXI_DRAM_TYPE_LPDDR2 ||
	    para->dram_type == SUNXI_DRAM_TYPE_LPDDR3) {
		if (dqs_gating_mode == 1)
			clrsetbits_le32(0x310311c, 0x080000c0, 0x80000000);
		else
			clrsetbits_le32(0x310311c, 0x77000000, 0x22000000);
	}

	clrsetbits_le32(0x31030c0, 0x0fffffff,
			(config->dram_para2 & BIT(12)) ? 0x03000001 : 0x01000007);

	if (readl(0x70005d4) & BIT(16)) {
		clrbits_le32(0x7010250, 0x2);
		udelay(10);
	}

	// Set ZQ config
	clrsetbits_le32(0x3103140, 0x3ffffff,
			(para->dram_zq & 0x00ffffff) | BIT(25));

	// Initialise DRAM controller
	if (dqs_gating_mode == 1) {
		//writel(0x52, 0x3103000); // prep PHY reset + PLL init + z-cal
		writel(0x53, 0x3103000); // Go

		while ((readl(0x3103010) & 0x1) == 0) {
		} // wait for IDONE
		udelay(10);

		// 0x520 = prep DQS gating + DRAM init + d-cal
		if (para->dram_type == SUNXI_DRAM_TYPE_DDR3)
			writel(0x5a0, 0x3103000);		// + DRAM reset
		else
			writel(0x520, 0x3103000);
	} else {
		if ((readl(0x70005d4) & (1 << 16)) == 0) {
			// prep DRAM init + PHY reset + d-cal + PLL init + z-cal
			if (para->dram_type == SUNXI_DRAM_TYPE_DDR3)
				writel(0x1f2, 0x3103000);	// + DRAM reset
			else
				writel(0x172, 0x3103000);
		} else {
			// prep PHY reset + d-cal + z-cal
			writel(0x62, 0x3103000);
		}
	}

	setbits_le32(0x3103000, 0x1);		 // GO

	udelay(10);
	while ((readl(0x3103010) & 0x1) == 0) {
	} // wait for IDONE

	if (readl(0x70005d4) & BIT(16)) {
		clrsetbits_le32(0x310310c, 0x06000000, 0x04000000);
		udelay(10);

		setbits_le32(0x3103004, 0x1);

		while ((readl(0x3103018) & 0x7) != 0x3) {
		}

		clrbits_le32(0x7010250, 0x1);
		udelay(10);

		clrbits_le32(0x3103004, 0x1);

		while ((readl(0x3103018) & 0x7) != 0x1) {
		}

		udelay(15);

		if (dqs_gating_mode == 1) {
			clrbits_le32(0x3103108, 0xc0);
			clrsetbits_le32(0x310310c, 0x06000000, 0x02000000);
			udelay(1);
			writel(0x401, 0x3103000);

			while ((readl(0x3103010) & 0x1) == 0) {
			}
		}
	}

	// Check for training error
	if (readl(0x3103010) & BIT(20)) {
		printf("ZQ calibration error, check external 240 ohm resistor\n");
		return 0;
	}

	// STATR = Zynq STAT? Wait for status 'normal'?
	while ((readl(0x3103018) & 0x1) == 0) {
	}

	setbits_le32(0x310308c, BIT(31));
	udelay(10);
	clrbits_le32(0x310308c, BIT(31));
	udelay(10);
	setbits_le32(0x3102014, BIT(31));
	udelay(10);

	clrbits_le32(0x310310c, 0x06000000);

	if (dqs_gating_mode == 1)
		clrsetbits_le32(0x310311c, 0xc0, 0x40);

	return 1;
}

static unsigned int calculate_rank_size(uint32_t regval)
{
	unsigned int bits;

	bits = (regval >> 8) & 0xf;	/* page size - 3 */
	bits += (regval >> 4) & 0xf;	/* row width - 1 */
	bits += (regval >> 2) & 0x3;	/* bank count - 2 */
	bits -= 14;			/* 1MB = 20 bits, minus above 6 = 14 */

	return 1U << bits;
}

/*
 * The below routine reads the dram config registers and extracts
 * the number of address bits in each rank available. It then calculates
 * total memory size in MB.
 */
static unsigned int DRAMC_get_dram_size(void)
{
	uint32_t val;
	unsigned int size;

	val = readl(0x3102000);		/* MC_WORK_MODE0 */
	size = calculate_rank_size(val);
	if ((val & 0x3) == 0)		/* single rank? */
		return size;

	val = readl(0x3102004);		/* MC_WORK_MODE1 */
	if ((val & 0x3) == 0)		/* two identical ranks? */
		return size * 2;

	/* add sizes of both ranks */
	return size + calculate_rank_size(val);
}

/*
 * The below routine reads the command status register to extract
 * DQ width and rank count. This follows the DQS training command in
 * channel_init. If error bit 22 is reset, we have two ranks and full DQ.
 * If there was an error, figure out whether it was half DQ, single rank,
 * or both. Set bit 12 and 0 in dram_para2 with the results.
 */
static int dqs_gate_detect(dram_config_t *config)
{
	uint32_t dx0, dx1;

	if ((readl(0x3103010) & BIT(22)) == 0) {
		config->dram_para2 = (config->dram_para2 & ~0xf) | BIT(12);
		debug("dual rank and full DQ\n");

		return 1;
	}

	dx0 = (readl(0x3103348) & 0x3000000) >> 24;
	if (dx0 == 0) {
		config->dram_para2 = (config->dram_para2 & ~0xf) | 0x1001;
		debug("dual rank and half DQ\n");

		return 1;
	}

	if (dx0 == 2) {
		dx1 = (readl(0x31033c8) & 0x3000000) >> 24;
		if (dx1 == 2) {
			config->dram_para2 = config->dram_para2 & ~0xf00f;
			debug("single rank and full DQ\n");
		} else {
			config->dram_para2 = (config->dram_para2 & ~0xf00f) | BIT(0);
			debug("single rank and half DQ\n");
		}

		return 1;
	}

	if ((config->dram_tpr13 & BIT(29)) == 0)
		return 0;

	debug("DX0 state: %d\n", dx0);
	debug("DX1 state: %d\n", dx1);

	return 0;
}

static int dramc_simple_wr_test(unsigned int mem_mb, int len)
{
	unsigned int  offs	= (mem_mb / 2) << 18; // half of memory size
	unsigned int  patt1 = 0x01234567;
	unsigned int  patt2 = 0xfedcba98;
	unsigned int *addr, v1, v2, i;

	addr = (unsigned int *)CFG_SYS_SDRAM_BASE;
	for (i = 0; i != len; i++, addr++) {
		writel(patt1 + i, (unsigned long)addr);
		writel(patt2 + i, (unsigned long)(addr + offs));
	}

	addr = (unsigned int *)CFG_SYS_SDRAM_BASE;
	for (i = 0; i != len; i++) {
		v1 = readl((unsigned long)(addr + i));
		v2 = patt1 + i;
		if (v1 != v2) {
			printf("DRAM: simple test FAIL\n");
			printf("%x != %x at address %p\n", v1, v2, addr + i);
			return 1;
		}
		v1 = readl((unsigned long)(addr + offs + i));
		v2 = patt2 + i;
		if (v1 != v2) {
			printf("DRAM: simple test FAIL\n");
			printf("%x != %x at address %p\n", v1, v2, addr + offs + i);
			return 1;
		}
	}

	debug("DRAM: simple test OK\n");
	return 0;
}

// Set the Vref mode for the controller
//
static void mctl_vrefzq_init(const dram_para_t *para, const dram_config_t *config)
{
	if (config->dram_tpr13 & BIT(17))
		return;

	clrsetbits_le32(0x3103110, 0x7f7f7f7f, para->dram_tpr5);

	// IOCVR1
	if ((config->dram_tpr13 & BIT(16)) == 0)
		clrsetbits_le32(0x3103114, 0x7f, para->dram_tpr6 & 0x7f);
}

// Perform an init of the controller. This is actually done 3 times. The first
// time to establish the number of ranks and DQ width. The second time to
// establish the actual ram size. The third time is final one, with the final
// settings.
//
static int mctl_core_init(const dram_para_t *para, const dram_config_t *config)
{
	mctl_sys_init(para, config);

	mctl_vrefzq_init(para, config);

	mctl_com_init(para, config);

	mctl_phy_ac_remapping(para, config);

	mctl_set_timing_params(para, config);

	return mctl_channel_init(0, para, config);
}

/*
 * This routine sizes a DRAM device by cycling through address lines and
 * figuring out if they are connected to a real address line, or if the
 * address is a mirror.
 * First the column and bank bit allocations are set to low values (2 and 9
 * address lines). Then a maximum allocation (16 lines) is set for rows and
 * this is tested.
 * Next the BA2 line is checked. This seems to be placed above the column,
 * BA0-1 and row addresses. Finally, the column address is allocated 13 lines
 * and these are tested. The results are placed in dram_para1 and dram_para2.
 */

static uint32_t get_payload(bool odd, unsigned long int ptr)
{
	if (odd)
		return (uint32_t)ptr;
	else
		return ~((uint32_t)ptr);
}

static int auto_scan_dram_size(const dram_para_t *para, dram_config_t *config)
{
	unsigned int rval, i, j, rank, maxrank, offs;
	unsigned int shft;
	unsigned long ptr, mc_work_mode, chk;

	if (mctl_core_init(para, config) == 0) {
		printf("DRAM initialisation error : 0\n");
		return 0;
	}

	maxrank	= (config->dram_para2 & 0xf000) ? 2 : 1;
	mc_work_mode = 0x3102000;
	offs = 0;

	/* write test pattern */
	for (i = 0, ptr = CFG_SYS_SDRAM_BASE; i < 64; i++, ptr += 4)
		writel(get_payload(i & 0x1, ptr), ptr);

	for (rank = 0; rank < maxrank;) {
		/* set row mode */
		clrsetbits_le32(mc_work_mode, 0xf0c, 0x6f0);
		udelay(1);

		// Scan per address line, until address wraps (i.e. see shadow)
		for (i = 11; i < 17; i++) {
			chk = CFG_SYS_SDRAM_BASE + (1U << (i + 11));
			ptr = CFG_SYS_SDRAM_BASE;
			for (j = 0; j < 64; j++) {
				if (readl(chk) != get_payload(j & 0x1, ptr))
					break;
				ptr += 4;
				chk += 4;
			}
			if (j == 64)
				break;
		}
		if (i > 16)
			i = 16;
		debug("rank %d row = %d\n", rank, i);

		/* Store rows in para 1 */
		shft = offs + 4;
		rval = config->dram_para1;
		rval &= ~(0xff << shft);
		rval |= i << shft;
		config->dram_para1 = rval;

		if (rank == 1)		/* Set bank mode for rank0 */
			clrsetbits_le32(0x3102000, 0xffc, 0x6a4);

		/* Set bank mode for current rank */
		clrsetbits_le32(mc_work_mode, 0xffc, 0x6a4);
		udelay(1);

		// Test if bit A23 is BA2 or mirror XXX A22?
		chk = CFG_SYS_SDRAM_BASE + (1U << 22);
		ptr = CFG_SYS_SDRAM_BASE;
		for (i = 0, j = 0; i < 64; i++) {
			if (readl(chk) != get_payload(i & 1, ptr)) {
				j = 1;
				break;
			}
			ptr += 4;
			chk += 4;
		}

		debug("rank %d bank = %d\n", rank, (j + 1) << 2); /* 4 or 8 */

		/* Store banks in para 1 */
		shft = 12 + offs;
		rval = config->dram_para1;
		rval &= ~(0xf << shft);
		rval |= j << shft;
		config->dram_para1 = rval;

		if (rank == 1)		/* Set page mode for rank0 */
			clrsetbits_le32(0x3102000, 0xffc, 0xaa0);

		/* Set page mode for current rank */
		clrsetbits_le32(mc_work_mode, 0xffc, 0xaa0);
		udelay(1);

		// Scan per address line, until address wraps (i.e. see shadow)
		for (i = 9; i < 14; i++) {
			chk = CFG_SYS_SDRAM_BASE + (1U << i);
			ptr = CFG_SYS_SDRAM_BASE;
			for (j = 0; j < 64; j++) {
				if (readl(chk) != get_payload(j & 1, ptr))
					break;
				ptr += 4;
				chk += 4;
			}
			if (j == 64)
				break;
		}
		if (i > 13)
			i = 13;

		unsigned int pgsize = (i == 9) ? 0 : (1 << (i - 10));
		debug("rank %d page size = %d KB\n", rank, pgsize);

		/* Store page size */
		shft = offs;
		rval = config->dram_para1;
		rval &= ~(0xf << shft);
		rval |= pgsize << shft;
		config->dram_para1 = rval;

		// Move to next rank
		rank++;
		if (rank != maxrank) {
			if (rank == 1) {
				/* MC_WORK_MODE */
				clrsetbits_le32(0x3202000, 0xffc, 0x6f0);

				/* MC_WORK_MODE2 */
				clrsetbits_le32(0x3202004, 0xffc, 0x6f0);
			}
			/* store rank1 config in upper half of para1 */
			offs += 16;
			mc_work_mode += 4;	/* move to MC_WORK_MODE2 */
		}
	}
	if (maxrank == 2) {
		config->dram_para2 &= 0xfffff0ff;
		/* note: rval is equal to para->dram_para1 here */
		if ((rval & 0xffff) == (rval >> 16)) {
			debug("rank1 config same as rank0\n");
		} else {
			config->dram_para2 |= BIT(8);
			debug("rank1 config different from rank0\n");
		}
	}

	return 1;
}

/*
 * This routine sets up parameters with dqs_gating_mode equal to 1 and two
 * ranks enabled. It then configures the core and tests for 1 or 2 ranks and
 * full or half DQ width. It then resets the parameters to the original values.
 * dram_para2 is updated with the rank and width findings.
 */
static int auto_scan_dram_rank_width(const dram_para_t *para,
				     dram_config_t *config)
{
	unsigned int s1 = config->dram_tpr13;
	unsigned int s2 = config->dram_para1;

	config->dram_para1 = 0x00b000b0;
	config->dram_para2 = (config->dram_para2 & ~0xf) | BIT(12);

	/* set DQS probe mode */
	config->dram_tpr13 = (config->dram_tpr13 & ~0x8) | BIT(2) | BIT(0);

	mctl_core_init(para, config);

	if (readl(0x3103010) & BIT(20))
		return 0;

	if (dqs_gate_detect(config) == 0)
		return 0;

	config->dram_tpr13 = s1;
	config->dram_para1 = s2;

	return 1;
}

/*
 * This routine determines the SDRAM topology. It first establishes the number
 * of ranks and the DQ width. Then it scans the SDRAM address lines to establish
 * the size of each rank. It then updates dram_tpr13 to reflect that the sizes
 * are now known: a re-init will not repeat the autoscan.
 */
static int auto_scan_dram_config(const dram_para_t *para,
				 dram_config_t *config)
{
	if (((config->dram_tpr13 & BIT(14)) == 0) &&
	    (auto_scan_dram_rank_width(para, config) == 0)) {
		printf("ERROR: auto scan dram rank & width failed\n");
		return 0;
	}

	if (((config->dram_tpr13 & BIT(0)) == 0) &&
	    (auto_scan_dram_size(para, config) == 0)) {
		printf("ERROR: auto scan dram size failed\n");
		return 0;
	}

	if ((config->dram_tpr13 & BIT(15)) == 0)
		config->dram_tpr13 |= BIT(14) | BIT(13) | BIT(1) | BIT(0);

	return 1;
}

static int init_DRAM(int type, const dram_para_t *para)
{
	dram_config_t config = {
		.dram_para1	= 0x000010d2,
		.dram_para2	= 0,
		.dram_tpr13	= CONFIG_DRAM_SUNXI_TPR13,
	};
	u32 rc, mem_size_mb;

	debug("DRAM BOOT DRIVE INFO: %s\n", "V0.24");
	debug("DRAM CLK = %d MHz\n", para->dram_clk);
	debug("DRAM Type = %d (2:DDR2,3:DDR3)\n", para->dram_type);
	if ((para->dram_odt_en & 0x1) == 0)
		debug("DRAMC read ODT off\n");
	else
		debug("DRAMC ZQ value: 0x%x\n", para->dram_zq);

	/* Test ZQ status */
	if (config.dram_tpr13 & BIT(16)) {
		debug("DRAM only have internal ZQ\n");
		setbits_le32(0x3000160, BIT(8));
		writel(0, 0x3000168);
		udelay(10);
	} else {
		clrbits_le32(0x3000160, 0x3);
		writel(config.dram_tpr13 & BIT(16), 0x7010254);
		udelay(10);
		clrsetbits_le32(0x3000160, 0x108, BIT(1));
		udelay(10);
		setbits_le32(0x3000160, BIT(0));
		udelay(20);
		debug("ZQ value = 0x%x\n", readl(0x300016c));
	}

	dram_voltage_set(para);

	/* Set SDRAM controller auto config */
	if ((config.dram_tpr13 & BIT(0)) == 0) {
		if (auto_scan_dram_config(para, &config) == 0) {
			printf("auto_scan_dram_config() FAILED\n");
			return 0;
		}
	}

	/* report ODT */
	rc = para->dram_mr1;
	if ((rc & 0x44) == 0)
		debug("DRAM ODT off\n");
	else
		debug("DRAM ODT value: 0x%x\n", rc);

	/* Init core, final run */
	if (mctl_core_init(para, &config) == 0) {
		printf("DRAM initialisation error: 1\n");
		return 0;
	}

	/* Get SDRAM size */
	/* TODO: who ever puts a negative number in the top half? */
	rc = config.dram_para2;
	if (rc & BIT(31)) {
		rc = (rc >> 16) & ~BIT(15);
	} else {
		rc = DRAMC_get_dram_size();
		debug("DRAM: size = %dMB\n", rc);
		config.dram_para2 = (config.dram_para2 & 0xffffU) | rc << 16;
	}
	mem_size_mb = rc;

	/* Purpose ?? */
	if (config.dram_tpr13 & BIT(30)) {
		rc = para->dram_tpr8;
		if (rc == 0)
			rc = 0x10000200;
		writel(rc, 0x31030a0);
		writel(0x40a, 0x310309c);
		setbits_le32(0x3103004, BIT(0));
		debug("Enable Auto SR\n");
	} else {
		clrbits_le32(0x31030a0, 0xffff);
		clrbits_le32(0x3103004, 0x1);
	}

	/* Purpose ?? */
	if (config.dram_tpr13 & BIT(9)) {
		clrsetbits_le32(0x3103100, 0xf000, 0x5000);
	} else {
		if (para->dram_type != SUNXI_DRAM_TYPE_LPDDR2)
			clrbits_le32(0x3103100, 0xf000);
	}

	setbits_le32(0x3103140, BIT(31));

	/* CHECK: is that really writing to a different register? */
	if (config.dram_tpr13 & BIT(8))
		writel(readl(0x3103140) | 0x300, 0x31030b8);

	if (config.dram_tpr13 & BIT(16))
		clrbits_le32(0x3103108, BIT(13));
	else
		setbits_le32(0x3103108, BIT(13));

	/* Purpose ?? */
	if (para->dram_type == SUNXI_DRAM_TYPE_LPDDR3)
		clrsetbits_le32(0x310307c, 0xf0000, 0x1000);

	dram_enable_all_master();
	if (config.dram_tpr13 & BIT(28)) {
		if ((readl(0x70005d4) & BIT(16)) ||
		    dramc_simple_wr_test(mem_size_mb, 4096))
			return 0;
	}

	return mem_size_mb;
}

static const dram_para_t para = {
	.dram_clk	= CONFIG_DRAM_CLK,
	.dram_type	= CONFIG_SUNXI_DRAM_TYPE,
	.dram_zq	= CONFIG_DRAM_ZQ,
	.dram_odt_en	= CONFIG_DRAM_SUNXI_ODT_EN,
	.dram_mr0	= 0x1c70,
	.dram_mr1	= 0x42,
	.dram_mr2	= 0x18,
	.dram_mr3	= 0,
	.dram_tpr0	= 0x004a2195,
	.dram_tpr1	= 0x02423190,
	.dram_tpr2	= 0x0008b061,
	.dram_tpr3	= 0xb4787896, // unused
	.dram_tpr4	= 0,
	.dram_tpr5	= 0x48484848,
	.dram_tpr6	= 0x00000048,
	.dram_tpr7	= 0x1620121e, // unused
	.dram_tpr8	= 0,
	.dram_tpr9	= 0, // clock?
	.dram_tpr10	= 0,
	.dram_tpr11	= CONFIG_DRAM_SUNXI_TPR11,
	.dram_tpr12	= CONFIG_DRAM_SUNXI_TPR12,
};

unsigned long sunxi_dram_init(void)
{
	return init_DRAM(0, &para) * 1024UL * 1024;
};

#ifdef CONFIG_RAM		/* using the driver model */
struct sunxi_ram_priv {
	size_t size;
};

static int sunxi_ram_probe(struct udevice *dev)
{
	struct sunxi_ram_priv *priv = dev_get_priv(dev);
	unsigned long dram_size;

	debug("%s: %s: probing\n", __func__, dev->name);

	dram_size = sunxi_dram_init();
	if (!dram_size) {
		printf("DRAM init failed\n");
		return -ENODEV;
	}

	priv->size = dram_size;

	return 0;
}

static int sunxi_ram_get_info(struct udevice *dev, struct ram_info *info)
{
	struct sunxi_ram_priv *priv = dev_get_priv(dev);

	debug("%s: %s: getting info\n", __func__, dev->name);

	info->base = CFG_SYS_SDRAM_BASE;
	info->size = priv->size;

	return 0;
}

static struct ram_ops sunxi_ram_ops = {
	.get_info = sunxi_ram_get_info,
};

static const struct udevice_id sunxi_ram_ids[] = {
	{ .compatible = "allwinner,sun20i-d1-mbus" },
	{ }
};

U_BOOT_DRIVER(sunxi_ram) = {
	.name = "sunxi_ram",
	.id = UCLASS_RAM,
	.of_match = sunxi_ram_ids,
	.ops = &sunxi_ram_ops,
	.probe = sunxi_ram_probe,
	.priv_auto = sizeof(struct sunxi_ram_priv),
};
#endif				/* CONFIG_RAM (using driver model) */
