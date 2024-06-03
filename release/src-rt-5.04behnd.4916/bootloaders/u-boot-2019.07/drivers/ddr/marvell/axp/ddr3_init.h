/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 */

#ifndef __DDR3_INIT_H
#define __DDR3_INIT_H

/*
 * Debug
 */

/*
 * MV_DEBUG_INIT need to be defines, otherwise the output of the
 * DDR2 training code is not complete and misleading
 */
#define MV_DEBUG_INIT

#ifdef MV_DEBUG_INIT
#define DEBUG_INIT_S(s)			puts(s)
#define DEBUG_INIT_D(d, l)		printf("%x", d)
#define DEBUG_INIT_D_10(d, l)		printf("%d", d)
#else
#define DEBUG_INIT_S(s)
#define DEBUG_INIT_D(d, l)
#define DEBUG_INIT_D_10(d, l)
#endif

#ifdef MV_DEBUG_INIT_FULL
#define DEBUG_INIT_FULL_S(s)		puts(s)
#define DEBUG_INIT_FULL_D(d, l)		printf("%x", d)
#define DEBUG_INIT_FULL_D_10(d, l)	printf("%d", d)
#define DEBUG_WR_REG(reg, val) \
	{ DEBUG_INIT_S("Write Reg: 0x"); DEBUG_INIT_D((reg), 8); \
	  DEBUG_INIT_S("= "); DEBUG_INIT_D((val), 8); DEBUG_INIT_S("\n"); }
#define DEBUG_RD_REG(reg, val) \
	{ DEBUG_INIT_S("Read  Reg: 0x"); DEBUG_INIT_D((reg), 8); \
	  DEBUG_INIT_S("= "); DEBUG_INIT_D((val), 8); DEBUG_INIT_S("\n"); }
#else
#define DEBUG_INIT_FULL_S(s)
#define DEBUG_INIT_FULL_D(d, l)
#define DEBUG_INIT_FULL_D_10(d, l)
#define DEBUG_WR_REG(reg, val)
#define DEBUG_RD_REG(reg, val)
#endif

#define DEBUG_INIT_FULL_C(s, d, l) \
	{ DEBUG_INIT_FULL_S(s); DEBUG_INIT_FULL_D(d, l); DEBUG_INIT_FULL_S("\n"); }
#define DEBUG_INIT_C(s, d, l) \
	{ DEBUG_INIT_S(s); DEBUG_INIT_D(d, l); DEBUG_INIT_S("\n"); }

#define MV_MBUS_REGS_OFFSET                 (0x20000)

#include "ddr3_hw_training.h"

#define MAX_DIMM_NUM			2
#define SPD_SIZE			128

#ifdef MV88F78X60
#include "ddr3_axp.h"
#elif defined(MV88F67XX)
#include "ddr3_a370.h"
#elif defined(MV88F672X)
#include "ddr3_a375.h"
#endif

/* DRR training Error codes */
/* Stage 0 errors */
#define MV_DDR3_TRAINING_ERR_BAD_SAR			0xDD300001
/* Stage 1 errors */
#define MV_DDR3_TRAINING_ERR_TWSI_FAIL			0xDD301001
#define MV_DDR3_TRAINING_ERR_DIMM_TYPE_NO_MATCH		0xDD301001
#define MV_DDR3_TRAINING_ERR_TWSI_BAD_TYPE		0xDD301003
#define MV_DDR3_TRAINING_ERR_BUS_WIDTH_NOT_MATCH	0xDD301004
#define MV_DDR3_TRAINING_ERR_BAD_DIMM_SETUP		0xDD301005
#define MV_DDR3_TRAINING_ERR_MAX_CS_LIMIT		0xDD301006
#define MV_DDR3_TRAINING_ERR_MAX_ENA_CS_LIMIT		0xDD301007
#define MV_DDR3_TRAINING_ERR_BAD_R_DIMM_SETUP		0xDD301008
/* Stage 2 errors */
#define MV_DDR3_TRAINING_ERR_HW_FAIL_BASE		0xDD302000

typedef enum config_type {
	CONFIG_ECC,
	CONFIG_MULTI_CS,
	CONFIG_BUS_WIDTH
} MV_CONFIG_TYPE;

enum log_level  {
	MV_LOG_LEVEL_0,
	MV_LOG_LEVEL_1,
	MV_LOG_LEVEL_2,
	MV_LOG_LEVEL_3
};

int ddr3_hw_training(u32 target_freq, u32 ddr_width,
		     int xor_bypass, u32 scrub_offs, u32 scrub_size,
		     int dqs_clk_aligned, int debug_mode, int reg_dimm_skip_wl);

void ddr3_print_version(void);
void fix_pll_val(u8 target_fab);
u8 ddr3_get_eprom_fabric(void);
u32 ddr3_get_fab_opt(void);
u32 ddr3_get_cpu_freq(void);
u32 ddr3_get_vco_freq(void);
int ddr3_check_config(u32 addr, MV_CONFIG_TYPE config_type);
u32 ddr3_get_static_mc_value(u32 reg_addr, u32 offset1, u32 mask1, u32 offset2,
			     u32 mask2);
u32 ddr3_cl_to_valid_cl(u32 cl);
u32 ddr3_valid_cl_to_cl(u32 ui_valid_cl);
u32 ddr3_get_cs_num_from_reg(void);
u32 ddr3_get_cs_ena_from_reg(void);
u8 mv_ctrl_rev_get(void);

u32 ddr3_get_log_level(void);

/* SPD */
int ddr3_dunit_setup(u32 ecc_ena, u32 hclk_time, u32 *ddr_width);

/*
 * Accessor functions for the registers
 */
static inline void reg_write(u32 addr, u32 val)
{
	writel(val, INTER_REGS_BASE + addr);
}

static inline u32 reg_read(u32 addr)
{
	return readl(INTER_REGS_BASE + addr);
}

static inline void reg_bit_set(u32 addr, u32 mask)
{
	setbits_le32(INTER_REGS_BASE + addr, mask);
}

static inline void reg_bit_clr(u32 addr, u32 mask)
{
	clrbits_le32(INTER_REGS_BASE + addr, mask);
}

#endif /* __DDR3_INIT_H */
