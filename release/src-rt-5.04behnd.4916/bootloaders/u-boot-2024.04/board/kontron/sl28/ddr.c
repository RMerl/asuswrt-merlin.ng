// SPDX-License-Identifier: GPL-2.0+

#include <common.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <fsl_ddr_sdram.h>

DECLARE_GLOBAL_DATA_PTR;

#define DCFG_GPPORCR1 0x20

#define GPPORCR1_MEM_MASK		(0x7 << 5)
#define GPPORCR1_MEM_512MB_CS0		(0x0 << 5)
#define GPPORCR1_MEM_1GB_CS0		(0x1 << 5)
#define GPPORCR1_MEM_2GB_CS0		(0x2 << 5)
#define GPPORCR1_MEM_4GB_CS0_1		(0x3 << 5)
#define GPPORCR1_MEM_4GB_CS0_2		(0x4 << 5)
#define GPPORCR1_MEM_8GB_CS0_1_2_3	(0x5 << 5)
#define GPPORCR1_MEM_8GB_CS0_1		(0x6 << 5)

static fsl_ddr_cfg_regs_t __maybe_unused ddr_cfg_regs = {
	.cs[0].bnds		= 0x0000007f,
	.cs[0].config		= 0x80044402,
	.cs[1].bnds		= 0x008000ff,
	.cs[1].config		= 0x80004402,

	.timing_cfg_0		= 0x9011010c,
	.timing_cfg_3		= 0x010c1000,
	.timing_cfg_1		= 0xbcb48c66,
	.timing_cfg_2		= 0x0fc0d118,
	.ddr_sdram_cfg		= 0xe70c000c,
	.ddr_sdram_cfg_2	= 0x24401111,
	.ddr_sdram_mode		= 0x00441c70,
	.ddr_sdram_mode_3	= 0x00001c70,
	.ddr_sdram_mode_5	= 0x00001c70,
	.ddr_sdram_mode_7	= 0x00001c70,
	.ddr_sdram_mode_2	= 0x00180000,
	.ddr_sdram_mode_4	= 0x00180000,
	.ddr_sdram_mode_6	= 0x00180000,
	.ddr_sdram_mode_8	= 0x00180000,

	.ddr_sdram_interval	= 0x0c30030c,
	.ddr_data_init		= 0xdeadbeef,

	.ddr_sdram_clk_cntl	= 0x02400000,

	.timing_cfg_4		= 0x00000001,
	.timing_cfg_5		= 0x04401400,

	.ddr_zq_cntl		= 0x89080600,
	.ddr_wrlvl_cntl		= 0x8675f606,
	.ddr_wrlvl_cntl_2	= 0x04080700,
	.ddr_wrlvl_cntl_3	= 0x00000009,

	.ddr_cdr1		= 0x80040000,
	.ddr_cdr2		= 0x0000bc01,

	/* Erratum A-009942, set optimal CPO value */
	.debug[28]		= 0x00700040,
};

int fsl_initdram(void)
{
	u32 gpporcr1 = in_le32(DCFG_BASE + DCFG_GPPORCR1);
	phys_size_t dram_size;

	switch (gpporcr1 & GPPORCR1_MEM_MASK) {
	case GPPORCR1_MEM_2GB_CS0:
		dram_size = 0x80000000;
		ddr_cfg_regs.cs[1].bnds = 0;
		ddr_cfg_regs.cs[1].config = 0;
		break;
	case GPPORCR1_MEM_4GB_CS0_1:
		dram_size = 0x100000000ULL;
		break;
	case GPPORCR1_MEM_8GB_CS0_1:
		dram_size = 0x200000000ULL;
		ddr_cfg_regs.cs[0].bnds = 0x000000ff;
		ddr_cfg_regs.cs[0].config = 0x80044403;
		ddr_cfg_regs.cs[1].bnds = 0x010001ff;
		ddr_cfg_regs.cs[1].config = 0x80044403;
		break;
	case GPPORCR1_MEM_512MB_CS0:
		dram_size = 0x20000000;
		fallthrough; /* for now */
	case GPPORCR1_MEM_1GB_CS0:
		dram_size = 0x40000000;
		fallthrough; /* for now */
	case GPPORCR1_MEM_4GB_CS0_2:
		dram_size = 0x100000000ULL;
		fallthrough; /* for now */
	case GPPORCR1_MEM_8GB_CS0_1_2_3:
		dram_size = 0x200000000ULL;
		fallthrough; /* for now */
	default:
		panic("Unsupported memory configuration (%08x)\n",
		      gpporcr1 & GPPORCR1_MEM_MASK);
		break;
	}

	if (!IS_ENABLED(CONFIG_SPL) || IS_ENABLED(CONFIG_SPL_BUILD))
		fsl_ddr_set_memctl_regs(&ddr_cfg_regs, 0, 0);

	gd->ram_size = dram_size;

	return 0;
}
