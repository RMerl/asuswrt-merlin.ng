// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2006 Freescale Semiconductor, Inc.
 *                    Dave Liu <daveliu@freescale.com>
 *
 * Copyright (C) 2007 Logic Product Development, Inc.
 *                    Peter Barada <peterb@logicpd.com>
 *
 * Copyright (C) 2007 MontaVista Software, Inc.
 *                    Anton Vorontsov <avorontsov@ru.mvista.com>
 *
 * (C) Copyright 2008 - 2010
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 */

#include <common.h>
#include <ioports.h>
#include <mpc83xx.h>
#include <i2c.h>
#include <miiphy.h>
#include <asm/io.h>
#include <asm/mmu.h>
#include <asm/processor.h>
#include <pci.h>
#include <linux/libfdt.h>
#include <post.h>

#include "../common/common.h"

DECLARE_GLOBAL_DATA_PTR;

static uchar ivm_content[CONFIG_SYS_IVM_EEPROM_MAX_LEN];

const qe_iop_conf_t qe_iop_conf_tab[] = {
	/* port pin dir open_drain assign */
#if defined(CONFIG_ARCH_MPC8360)
	/* MDIO */
	{0,  1, 3, 0, 2}, /* MDIO */
	{0,  2, 1, 0, 1}, /* MDC */

	/* UCC4 - UEC */
	{1, 14, 1, 0, 1}, /* TxD0 */
	{1, 15, 1, 0, 1}, /* TxD1 */
	{1, 20, 2, 0, 1}, /* RxD0 */
	{1, 21, 2, 0, 1}, /* RxD1 */
	{1, 18, 1, 0, 1}, /* TX_EN */
	{1, 26, 2, 0, 1}, /* RX_DV */
	{1, 27, 2, 0, 1}, /* RX_ER */
	{1, 24, 2, 0, 1}, /* COL */
	{1, 25, 2, 0, 1}, /* CRS */
	{2, 15, 2, 0, 1}, /* TX_CLK - CLK16 */
	{2, 16, 2, 0, 1}, /* RX_CLK - CLK17 */

	/* DUART - UART2 */
	{5,  0, 1, 0, 2}, /* UART2_SOUT */
	{5,  2, 1, 0, 1}, /* UART2_RTS */
	{5,  3, 2, 0, 2}, /* UART2_SIN */
	{5,  1, 2, 0, 3}, /* UART2_CTS */
#elif !defined(CONFIG_ARCH_MPC8309)
	/* Local Bus */
	{0, 16, 1, 0, 3}, /* LA00 */
	{0, 17, 1, 0, 3}, /* LA01 */
	{0, 18, 1, 0, 3}, /* LA02 */
	{0, 19, 1, 0, 3}, /* LA03 */
	{0, 20, 1, 0, 3}, /* LA04 */
	{0, 21, 1, 0, 3}, /* LA05 */
	{0, 22, 1, 0, 3}, /* LA06 */
	{0, 23, 1, 0, 3}, /* LA07 */
	{0, 24, 1, 0, 3}, /* LA08 */
	{0, 25, 1, 0, 3}, /* LA09 */
	{0, 26, 1, 0, 3}, /* LA10 */
	{0, 27, 1, 0, 3}, /* LA11 */
	{0, 28, 1, 0, 3}, /* LA12 */
	{0, 29, 1, 0, 3}, /* LA13 */
	{0, 30, 1, 0, 3}, /* LA14 */
	{0, 31, 1, 0, 3}, /* LA15 */

	/* MDIO */
	{3,  4, 3, 0, 2}, /* MDIO */
	{3,  5, 1, 0, 2}, /* MDC */

	/* UCC4 - UEC */
	{1, 18, 1, 0, 1}, /* TxD0 */
	{1, 19, 1, 0, 1}, /* TxD1 */
	{1, 22, 2, 0, 1}, /* RxD0 */
	{1, 23, 2, 0, 1}, /* RxD1 */
	{1, 26, 2, 0, 1}, /* RxER */
	{1, 28, 2, 0, 1}, /* Rx_DV */
	{1, 30, 1, 0, 1}, /* TxEN */
	{1, 31, 2, 0, 1}, /* CRS */
	{3, 10, 2, 0, 3}, /* TxCLK->CLK17 */
#endif

	/* END of table */
	{0,  0, 0, 0, QE_IOP_TAB_END},
};

#if defined(CONFIG_SUVD3)
const uint upma_table[] = {
	0x1ffedc00, 0x0ffcdc80, 0x0ffcdc80, 0x0ffcdc04, /* Words 0 to 3 */
	0x0ffcdc00, 0xffffcc00, 0xffffcc01, 0xfffffc01, /* Words 4 to 7 */
	0xfffffc01, 0xfffffc01, 0xfffffc01, 0xfffffc01, /* Words 8 to 11 */
	0xfffffc01, 0xfffffc01, 0xfffffc01, 0xfffffc01, /* Words 12 to 15 */
	0xfffffc01, 0xfffffc01, 0xfffffc01, 0xfffffc01, /* Words 16 to 19 */
	0xfffffc01, 0xfffffc01, 0xfffffc01, 0xfffffc01, /* Words 20 to 23 */
	0x9cfffc00, 0x00fffc80, 0x00fffc80, 0x00fffc00, /* Words 24 to 27 */
	0xffffec04, 0xffffec01, 0xfffffc01, 0xfffffc01, /* Words 28 to 31 */
	0xfffffc01, 0xfffffc01, 0xfffffc01, 0xfffffc01, /* Words 32 to 35 */
	0xfffffc01, 0xfffffc01, 0xfffffc01, 0xfffffc01, /* Words 36 to 39 */
	0xfffffc01, 0xfffffc01, 0xfffffc01, 0xfffffc01, /* Words 40 to 43 */
	0xfffffc01, 0xfffffc01, 0xfffffc01, 0xfffffc01, /* Words 44 to 47 */
	0xfffffc01, 0xfffffc01, 0xfffffc01, 0xfffffc01, /* Words 48 to 51 */
	0xfffffc01, 0xfffffc01, 0xfffffc01, 0xfffffc01, /* Words 52 to 55 */
	0xfffffc01, 0xfffffc01, 0xfffffc01, 0xfffffc01, /* Words 56 to 59 */
	0xfffffc01, 0xfffffc01, 0xfffffc01, 0xfffffc01  /* Words 60 to 63 */
};
#endif

static int piggy_present(void)
{
	struct km_bec_fpga __iomem *base =
		(struct km_bec_fpga __iomem *)CONFIG_SYS_KMBEC_FPGA_BASE;

	return in_8(&base->bprth) & PIGGY_PRESENT;
}

#if defined(CONFIG_KMVECT1)
int ethernet_present(void)
{
	/* ethernet port connected to simple switch without piggy */
	return 1;
}
#else
int ethernet_present(void)
{
	return piggy_present();
}
#endif


int board_early_init_r(void)
{
	struct km_bec_fpga *base =
		(struct km_bec_fpga *)CONFIG_SYS_KMBEC_FPGA_BASE;
#if defined(CONFIG_SUVD3)
	immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;
	fsl_lbc_t *lbc = &immap->im_lbc;
	u32 *mxmr = &lbc->mamr;
#endif

#if defined(CONFIG_ARCH_MPC8360)
	unsigned short	svid;
	/*
	 * Because of errata in the UCCs, we have to write to the reserved
	 * registers to slow the clocks down.
	 */
	svid =  SVR_REV(mfspr(SVR));
	switch (svid) {
	case 0x0020:
		/*
		 * MPC8360ECE.pdf QE_ENET10 table 4:
		 * IMMR + 0x14A8[4:5] = 11 (clk delay for UCC 2)
		 * IMMR + 0x14A8[18:19] = 11 (clk delay for UCC 1)
		 */
		setbits_be32((void *)(CONFIG_SYS_IMMR + 0x14a8), 0x0c003000);
		break;
	case 0x0021:
		/*
		 * MPC8360ECE.pdf QE_ENET10 table 4:
		 * IMMR + 0x14AC[24:27] = 1010
		 */
		clrsetbits_be32((void *)(CONFIG_SYS_IMMR + 0x14ac),
			0x00000050, 0x000000a0);
		break;
	}
#endif

	/* enable the PHY on the PIGGY */
	setbits_8(&base->pgy_eth, 0x01);
	/* enable the Unit LED (green) */
	setbits_8(&base->oprth, WRL_BOOT);
	/* enable Application Buffer */
	setbits_8(&base->oprtl, OPRTL_XBUFENA);

#if defined(CONFIG_SUVD3)
	/* configure UPMA for APP1 */
	upmconfig(UPMA, (uint *) upma_table,
		sizeof(upma_table) / sizeof(uint));
	out_be32(mxmr, CONFIG_SYS_MAMR);
#endif
	return 0;
}

int misc_init_r(void)
{
	ivm_read_eeprom(ivm_content, CONFIG_SYS_IVM_EEPROM_MAX_LEN);
	return 0;
}

#if defined(CONFIG_KMVECT1)
#include <mv88e6352.h>
/* Marvell MV88E6122 switch configuration */
static struct mv88e_sw_reg extsw_conf[] = {
	/* port 1, FRONT_MDI, autoneg */
	{ PORT(1), PORT_PHY, NO_SPEED_FOR },
	{ PORT(1), PORT_CTRL, FORWARDING | EGRS_FLD_ALL },
	{ PHY(1), PHY_1000_CTRL, NO_ADV },
	{ PHY(1), PHY_SPEC_CTRL, AUTO_MDIX_EN },
	{ PHY(1), PHY_CTRL, PHY_100_MBPS | AUTONEG_EN | AUTONEG_RST |
		FULL_DUPLEX },
	/* port 2, unused */
	{ PORT(2), PORT_CTRL, PORT_DIS },
	{ PHY(2), PHY_CTRL, PHY_PWR_DOWN },
	{ PHY(2), PHY_SPEC_CTRL, SPEC_PWR_DOWN },
	/* port 3, BP_MII (CPU), PHY mode, 100BASE */
	{ PORT(3), PORT_CTRL, FORWARDING | EGRS_FLD_ALL },
	/* port 4, ESTAR to slot 11, SerDes, 1000BASE-X */
	{ PORT(4), PORT_STATUS, NO_PHY_DETECT },
	{ PORT(4), PORT_PHY, SPEED_1000_FOR },
	{ PORT(4), PORT_CTRL, FORWARDING | EGRS_FLD_ALL },
	/* port 5, ESTAR to slot 13, SerDes, 1000BASE-X */
	{ PORT(5), PORT_STATUS, NO_PHY_DETECT },
	{ PORT(5), PORT_PHY, SPEED_1000_FOR },
	{ PORT(5), PORT_CTRL, FORWARDING | EGRS_FLD_ALL },
	/*
	 * Errata Fix: 1.9V Output from Internal 1.8V Regulator,
	 * acc . MV-S300889-00D.pdf , clause 4.5
	 */
	{ PORT(5), 0x1A, 0xADB1 },
	/* port 6, unused, this port has no phy */
	{ PORT(6), PORT_CTRL, PORT_DIS },
	/*
	 * Errata Fix: 1.9V Output from Internal 1.8V Regulator,
	 * acc . MV-S300889-00D.pdf , clause 4.5
	 */
	{ PORT(5), 0x1A, 0xADB1 },
};
#endif

int last_stage_init(void)
{
#if defined(CONFIG_KMVECT1)
	struct km_bec_fpga __iomem *base =
		(struct km_bec_fpga __iomem *)CONFIG_SYS_KMBEC_FPGA_BASE;
	u8 tmp_reg;

	/* Release mv88e6122 from reset */
	tmp_reg = in_8(&base->res1[0]) | 0x10; /* DIRECT3 register */
	out_8(&base->res1[0], tmp_reg);	       /* GP28 as output */
	tmp_reg = in_8(&base->gprt3) | 0x10;   /* GP28 to high */
	out_8(&base->gprt3, tmp_reg);

	/* configure MV88E6122 switch */
	char *name = "UEC2";

	if (miiphy_set_current_dev(name))
		return 0;

	mv88e_sw_program(name, CONFIG_KM_MVEXTSW_ADDR, extsw_conf,
		ARRAY_SIZE(extsw_conf));

	mv88e_sw_reset(name, CONFIG_KM_MVEXTSW_ADDR);

	if (piggy_present()) {
		env_set("ethact", "UEC2");
		env_set("netdev", "eth1");
		puts("using PIGGY for network boot\n");
	} else {
		env_set("netdev", "eth0");
		puts("using frontport for network boot\n");
	}
#endif

#if defined(CONFIG_TARGET_KMCOGE5NE)
	struct bfticu_iomap *base =
		(struct bfticu_iomap *)CONFIG_SYS_BFTIC3_BASE;
	u8 dip_switch = in_8((u8 *)&(base->mswitch)) & BFTICU_DIPSWITCH_MASK;

	if (dip_switch != 0) {
		/* start bootloader */
		puts("DIP:   Enabled\n");
		env_set("actual_bank", "0");
	}
#endif
	set_km_env();
	return 0;
}

static int fixed_sdram(void)
{
	immap_t *im = (immap_t *)CONFIG_SYS_IMMR;
	u32 msize = 0;
	u32 ddr_size;
	u32 ddr_size_log2;

	out_be32(&im->sysconf.ddrlaw[0].ar, (LAWAR_EN | 0x1e));
	out_be32(&im->ddr.csbnds[0].csbnds, (CONFIG_SYS_DDR_CS0_BNDS) | 0x7f);
	out_be32(&im->ddr.cs_config[0], CONFIG_SYS_DDR_CS0_CONFIG);
	out_be32(&im->ddr.timing_cfg_0, CONFIG_SYS_DDR_TIMING_0);
	out_be32(&im->ddr.timing_cfg_1, CONFIG_SYS_DDR_TIMING_1);
	out_be32(&im->ddr.timing_cfg_2, CONFIG_SYS_DDR_TIMING_2);
	out_be32(&im->ddr.timing_cfg_3, CONFIG_SYS_DDR_TIMING_3);
	out_be32(&im->ddr.sdram_cfg, CONFIG_SYS_DDR_SDRAM_CFG);
	out_be32(&im->ddr.sdram_cfg2, CONFIG_SYS_DDR_SDRAM_CFG2);
	out_be32(&im->ddr.sdram_mode, CONFIG_SYS_DDR_MODE);
	out_be32(&im->ddr.sdram_mode2, CONFIG_SYS_DDR_MODE2);
	out_be32(&im->ddr.sdram_interval, CONFIG_SYS_DDR_INTERVAL);
	out_be32(&im->ddr.sdram_clk_cntl, CONFIG_SYS_DDR_CLK_CNTL);
	udelay(200);
	setbits_be32(&im->ddr.sdram_cfg, SDRAM_CFG_MEM_EN);

	msize = CONFIG_SYS_DDR_SIZE << 20;
	disable_addr_trans();
	msize = get_ram_size(CONFIG_SYS_SDRAM_BASE, msize);
	enable_addr_trans();
	msize /= (1024 * 1024);
	if (CONFIG_SYS_DDR_SIZE != msize) {
		for (ddr_size = msize << 20, ddr_size_log2 = 0;
			(ddr_size > 1);
			ddr_size = ddr_size >> 1, ddr_size_log2++)
			if (ddr_size & 1)
				return -1;
		out_be32(&im->sysconf.ddrlaw[0].ar,
			(LAWAR_EN | ((ddr_size_log2 - 1) & LAWAR_SIZE)));
		out_be32(&im->ddr.csbnds[0].csbnds,
			(((msize / 16) - 1) & 0xff));
	}

	return msize;
}

int dram_init(void)
{
	immap_t *im = (immap_t *)CONFIG_SYS_IMMR;
	u32 msize = 0;

	if ((in_be32(&im->sysconf.immrbar) & IMMRBAR_BASE_ADDR) != (u32)im)
		return -ENXIO;

	out_be32(&im->sysconf.ddrlaw[0].bar,
		CONFIG_SYS_SDRAM_BASE & LAWBAR_BAR);
	msize = fixed_sdram();

#if defined(CONFIG_DDR_ECC) && !defined(CONFIG_ECC_INIT_VIA_DDRCONTROLLER)
	/*
	 * Initialize DDR ECC byte
	 */
	ddr_enable_ecc(msize * 1024 * 1024);
#endif

	/* return total bus SDRAM size(bytes)  -- DDR */
	gd->ram_size = msize * 1024 * 1024;

	return 0;
}

int checkboard(void)
{
	puts("Board: Keymile " CONFIG_KM_BOARD_NAME);

	if (piggy_present())
		puts(" with PIGGY.");
	puts("\n");
	return 0;
}

int ft_board_setup(void *blob, bd_t *bd)
{
	ft_cpu_setup(blob, bd);

	return 0;
}

#if defined(CONFIG_HUSH_INIT_VAR)
int hush_init_var(void)
{
	ivm_analyze_eeprom(ivm_content, CONFIG_SYS_IVM_EEPROM_MAX_LEN);
	return 0;
}
#endif

#if defined(CONFIG_POST)
int post_hotkeys_pressed(void)
{
	int testpin = 0;
	struct km_bec_fpga *base =
		(struct km_bec_fpga *)CONFIG_SYS_KMBEC_FPGA_BASE;
	int testpin_reg = in_8(&base->CONFIG_TESTPIN_REG);
	testpin = (testpin_reg & CONFIG_TESTPIN_MASK) != 0;
	debug("post_hotkeys_pressed: %d\n", !testpin);
	return testpin;
}

ulong post_word_load(void)
{
	void* addr = (ulong *) (CPM_POST_WORD_ADDR);
	debug("post_word_load 0x%08lX:  0x%08X\n", (ulong)addr, in_le32(addr));
	return in_le32(addr);

}
void post_word_store(ulong value)
{
	void* addr = (ulong *) (CPM_POST_WORD_ADDR);
	debug("post_word_store 0x%08lX: 0x%08lX\n", (ulong)addr, value);
	out_le32(addr, value);
}

int arch_memory_test_prepare(u32 *vstart, u32 *size, phys_addr_t *phys_offset)
{
	*vstart = CONFIG_SYS_MEMTEST_START;
	*size = CONFIG_SYS_MEMTEST_END - CONFIG_SYS_MEMTEST_START;
	debug("arch_memory_test_prepare 0x%08X 0x%08X\n", *vstart, *size);

	return 0;
}
#endif
