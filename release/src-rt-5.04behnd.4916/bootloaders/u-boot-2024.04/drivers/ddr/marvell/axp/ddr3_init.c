// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 */

#include <common.h>
#include <i2c.h>
#include <log.h>
#include <spl.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>
#include <linux/delay.h>

#include "ddr3_init.h"

#if defined(MV88F78X60)
#include "ddr3_axp_vars.h"
#elif defined(MV88F67XX)
#include "ddr3_a370_vars.h"
#elif defined(MV88F672X)
#include "ddr3_a375_vars.h"
#endif

#ifdef STATIC_TRAINING
static void ddr3_static_training_init(void);
#endif
#ifdef DUNIT_STATIC
static void ddr3_static_mc_init(void);
#endif
#if defined(DUNIT_STATIC) || defined(STATIC_TRAINING)
MV_DRAM_MODES *ddr3_get_static_ddr_mode(void);
#endif
#if defined(MV88F672X)
void get_target_freq(u32 freq_mode, u32 *ddr_freq, u32 *hclk_ps);
#endif
u32 mv_board_id_get(void);
extern void ddr3_set_sw_wl_rl_debug(u32);
extern void ddr3_set_pbs(u32);
extern void ddr3_set_log_level(u32 val);

static u32 log_level = DDR3_LOG_LEVEL;

static u32 ddr3_init_main(void);

/*
 * Name:     ddr3_set_log_level
 * Desc:     This routine initialize the log_level acording to nLogLevel
 *           which getting from user
 * Args:     nLogLevel
 * Notes:
 * Returns:  None.
 */
void ddr3_set_log_level(u32 val)
{
	log_level = val;
}

/*
 * Name:     ddr3_get_log_level
 * Desc:     This routine returns the log level
 * Args:     none
 * Notes:
 * Returns:  log level.
 */
u32 ddr3_get_log_level(void)
{
	return log_level;
}

static void debug_print_reg(u32 reg)
{
	printf("0x%08x = 0x%08x\n", reg, reg_read(reg));
}

static void print_dunit_setup(void)
{
	puts("\n########### LOG LEVEL 1 (D-UNIT SETUP)###########\n");

#ifdef DUNIT_STATIC
	puts("\nStatic D-UNIT Setup:\n");
#endif
#ifdef DUNIT_SPD
	puts("\nDynamic(using SPD) D-UNIT Setup:\n");
#endif
	debug_print_reg(REG_SDRAM_CONFIG_ADDR);
	debug_print_reg(REG_DUNIT_CTRL_LOW_ADDR);
	debug_print_reg(REG_SDRAM_TIMING_LOW_ADDR);
	debug_print_reg(REG_SDRAM_TIMING_HIGH_ADDR);
	debug_print_reg(REG_SDRAM_ADDRESS_CTRL_ADDR);
	debug_print_reg(REG_SDRAM_OPEN_PAGES_ADDR);
	debug_print_reg(REG_SDRAM_OPERATION_ADDR);
	debug_print_reg(REG_SDRAM_MODE_ADDR);
	debug_print_reg(REG_SDRAM_EXT_MODE_ADDR);
	debug_print_reg(REG_DDR_CONT_HIGH_ADDR);
	debug_print_reg(REG_ODT_TIME_LOW_ADDR);
	debug_print_reg(REG_SDRAM_ERROR_ADDR);
	debug_print_reg(REG_SDRAM_AUTO_PWR_SAVE_ADDR);
	debug_print_reg(REG_OUDDR3_TIMING_ADDR);
	debug_print_reg(REG_ODT_TIME_HIGH_ADDR);
	debug_print_reg(REG_SDRAM_ODT_CTRL_LOW_ADDR);
	debug_print_reg(REG_SDRAM_ODT_CTRL_HIGH_ADDR);
	debug_print_reg(REG_DUNIT_ODT_CTRL_ADDR);
#ifndef MV88F67XX
	debug_print_reg(REG_DRAM_FIFO_CTRL_ADDR);
	debug_print_reg(REG_DRAM_AXI_CTRL_ADDR);
	debug_print_reg(REG_DRAM_ADDR_CTRL_DRIVE_STRENGTH_ADDR);
	debug_print_reg(REG_DRAM_DATA_DQS_DRIVE_STRENGTH_ADDR);
	debug_print_reg(REG_DRAM_VER_CAL_MACHINE_CTRL_ADDR);
	debug_print_reg(REG_DRAM_MAIN_PADS_CAL_ADDR);
	debug_print_reg(REG_DRAM_HOR_CAL_MACHINE_CTRL_ADDR);
	debug_print_reg(REG_CS_SIZE_SCRATCH_ADDR);
	debug_print_reg(REG_DYNAMIC_POWER_SAVE_ADDR);
	debug_print_reg(REG_READ_DATA_SAMPLE_DELAYS_ADDR);
	debug_print_reg(REG_READ_DATA_READY_DELAYS_ADDR);
	debug_print_reg(REG_DDR3_MR0_ADDR);
	debug_print_reg(REG_DDR3_MR1_ADDR);
	debug_print_reg(REG_DDR3_MR2_ADDR);
	debug_print_reg(REG_DDR3_MR3_ADDR);
	debug_print_reg(REG_DDR3_RANK_CTRL_ADDR);
	debug_print_reg(REG_DRAM_PHY_CONFIG_ADDR);
	debug_print_reg(REG_STATIC_DRAM_DLB_CONTROL);
	debug_print_reg(DLB_BUS_OPTIMIZATION_WEIGHTS_REG);
	debug_print_reg(DLB_AGING_REGISTER);
	debug_print_reg(DLB_EVICTION_CONTROL_REG);
	debug_print_reg(DLB_EVICTION_TIMERS_REGISTER_REG);
#if defined(MV88F672X)
	debug_print_reg(REG_FASTPATH_WIN_CTRL_ADDR(0));
	debug_print_reg(REG_FASTPATH_WIN_BASE_ADDR(0));
	debug_print_reg(REG_FASTPATH_WIN_CTRL_ADDR(1));
	debug_print_reg(REG_FASTPATH_WIN_BASE_ADDR(1));
#else
	debug_print_reg(REG_FASTPATH_WIN_0_CTRL_ADDR);
#endif
	debug_print_reg(REG_CDI_CONFIG_ADDR);
#endif
}

#if !defined(STATIC_TRAINING)
static void ddr3_restore_and_set_final_windows(u32 *win_backup)
{
	u32 ui, reg, cs;
	u32 win_ctrl_reg, num_of_win_regs;
	u32 cs_ena = ddr3_get_cs_ena_from_reg();

#if defined(MV88F672X)
	if (DDR3_FAST_PATH_EN == 0)
		return;
#endif

#if defined(MV88F672X)
	win_ctrl_reg = REG_XBAR_WIN_16_CTRL_ADDR;
	num_of_win_regs = 8;
#else
	win_ctrl_reg = REG_XBAR_WIN_4_CTRL_ADDR;
	num_of_win_regs = 16;
#endif

	/* Return XBAR windows 4-7 or 16-19 init configuration */
	for (ui = 0; ui < num_of_win_regs; ui++)
		reg_write((win_ctrl_reg + 0x4 * ui), win_backup[ui]);

	DEBUG_INIT_FULL_S("DDR3 Training Sequence - Switching XBAR Window to FastPath Window\n");

#if defined(MV88F672X)
	/* Set L2 filtering to 1G */
	reg_write(0x8c04, 0x40000000);

	/* Open fast path windows */
	for (cs = 0; cs < MAX_CS; cs++) {
		if (cs_ena & (1 << cs)) {
			/* set fast path window control for the cs */
			reg = 0x1FFFFFE1;
			reg |= (cs << 2);
			reg |= (SDRAM_CS_SIZE & 0xFFFF0000);
			/* Open fast path Window */
			reg_write(REG_FASTPATH_WIN_CTRL_ADDR(cs), reg);
			/* set fast path window base address for the cs */
			reg = (((SDRAM_CS_SIZE + 1) * cs) & 0xFFFF0000);
			/* Set base address */
			reg_write(REG_FASTPATH_WIN_BASE_ADDR(cs), reg);
		}
	}
#else
	reg = 0x1FFFFFE1;
	for (cs = 0; cs < MAX_CS; cs++) {
		if (cs_ena & (1 << cs)) {
			reg |= (cs << 2);
			break;
		}
	}

	/* Open fast path Window to - 0.5G */
	reg_write(REG_FASTPATH_WIN_0_CTRL_ADDR, reg);
#endif
}

static void ddr3_save_and_set_training_windows(u32 *win_backup)
{
	u32 cs_ena = ddr3_get_cs_ena_from_reg();
	u32 reg, tmp_count, cs, ui;
	u32 win_ctrl_reg, win_base_reg, win_remap_reg;
	u32 num_of_win_regs, win_jump_index;

#if defined(MV88F672X)
	/* Disable L2 filtering */
	reg_write(0x8c04, 0);

	win_ctrl_reg = REG_XBAR_WIN_16_CTRL_ADDR;
	win_base_reg = REG_XBAR_WIN_16_BASE_ADDR;
	win_remap_reg = REG_XBAR_WIN_16_REMAP_ADDR;
	win_jump_index = 0x8;
	num_of_win_regs = 8;
#else
	win_ctrl_reg = REG_XBAR_WIN_4_CTRL_ADDR;
	win_base_reg = REG_XBAR_WIN_4_BASE_ADDR;
	win_remap_reg = REG_XBAR_WIN_4_REMAP_ADDR;
	win_jump_index = 0x10;
	num_of_win_regs = 16;
#endif

	/* Close XBAR Window 19 - Not needed */
	/* {0x000200e8}  -   Open Mbus Window - 2G */
	reg_write(REG_XBAR_WIN_19_CTRL_ADDR, 0);

	/* Save XBAR Windows 4-19 init configurations */
	for (ui = 0; ui < num_of_win_regs; ui++)
		win_backup[ui] = reg_read(win_ctrl_reg + 0x4 * ui);

	/* Open XBAR Windows 4-7 or 16-19 for other CS */
	reg = 0;
	tmp_count = 0;
	for (cs = 0; cs < MAX_CS; cs++) {
		if (cs_ena & (1 << cs)) {
			switch (cs) {
			case 0:
				reg = 0x0E00;
				break;
			case 1:
				reg = 0x0D00;
				break;
			case 2:
				reg = 0x0B00;
				break;
			case 3:
				reg = 0x0700;
				break;
			}
			reg |= (1 << 0);
			reg |= (SDRAM_CS_SIZE & 0xFFFF0000);

			reg_write(win_ctrl_reg + win_jump_index * tmp_count,
				  reg);
			reg = ((SDRAM_CS_SIZE + 1) * (tmp_count)) & 0xFFFF0000;
			reg_write(win_base_reg + win_jump_index * tmp_count,
				  reg);

			if (win_remap_reg <= REG_XBAR_WIN_7_REMAP_ADDR) {
				reg_write(win_remap_reg +
					  win_jump_index * tmp_count, 0);
			}

			tmp_count++;
		}
	}
}
#endif /*  !defined(STATIC_TRAINING) */

/*
 * Name:     ddr3_init - Main DDR3 Init function
 * Desc:     This routine initialize the DDR3 MC and runs HW training.
 * Args:     None.
 * Notes:
 * Returns:  None.
 */
int ddr3_init(void)
{
	unsigned int status;

	ddr3_set_pbs(DDR3_PBS);
	ddr3_set_sw_wl_rl_debug(DDR3_RUN_SW_WHEN_HW_FAIL);

	status = ddr3_init_main();
	if (status == MV_DDR3_TRAINING_ERR_BAD_SAR)
		DEBUG_INIT_S("DDR3 Training Error: Bad sample at reset");
	if (status == MV_DDR3_TRAINING_ERR_BAD_DIMM_SETUP)
		DEBUG_INIT_S("DDR3 Training Error: Bad DIMM setup");
	if (status == MV_DDR3_TRAINING_ERR_MAX_CS_LIMIT)
		DEBUG_INIT_S("DDR3 Training Error: Max CS limit");
	if (status == MV_DDR3_TRAINING_ERR_MAX_ENA_CS_LIMIT)
		DEBUG_INIT_S("DDR3 Training Error: Max enable CS limit");
	if (status == MV_DDR3_TRAINING_ERR_BAD_R_DIMM_SETUP)
		DEBUG_INIT_S("DDR3 Training Error: Bad R-DIMM setup");
	if (status == MV_DDR3_TRAINING_ERR_TWSI_FAIL)
		DEBUG_INIT_S("DDR3 Training Error: TWSI failure");
	if (status == MV_DDR3_TRAINING_ERR_DIMM_TYPE_NO_MATCH)
		DEBUG_INIT_S("DDR3 Training Error: DIMM type no match");
	if (status == MV_DDR3_TRAINING_ERR_TWSI_BAD_TYPE)
		DEBUG_INIT_S("DDR3 Training Error: TWSI bad type");
	if (status == MV_DDR3_TRAINING_ERR_BUS_WIDTH_NOT_MATCH)
		DEBUG_INIT_S("DDR3 Training Error: bus width no match");
	if (status > MV_DDR3_TRAINING_ERR_HW_FAIL_BASE)
		DEBUG_INIT_C("DDR3 Training Error: HW Failure 0x", status, 8);

	return status;
}

static void print_ddr_target_freq(u32 cpu_freq, u32 fab_opt)
{
	puts("\nDDR3 Training Sequence - Run DDR3 at ");

	switch (cpu_freq) {
#if defined(MV88F672X)
	case 21:
		puts("533 Mhz\n");
		break;
#else
	case 1:
		puts("533 Mhz\n");
		break;
	case 2:
		if (fab_opt == 5)
			puts("600 Mhz\n");
		if (fab_opt == 9)
			puts("400 Mhz\n");
		break;
	case 3:
		puts("667 Mhz\n");
		break;
	case 4:
		if (fab_opt == 5)
			puts("750 Mhz\n");
		if (fab_opt == 9)
			puts("500 Mhz\n");
		break;
	case 0xa:
		puts("400 Mhz\n");
		break;
	case 0xb:
		if (fab_opt == 5)
			puts("800 Mhz\n");
		if (fab_opt == 9)
			puts("553 Mhz\n");
		if (fab_opt == 0xA)
			puts("640 Mhz\n");
		break;
#endif
	default:
		puts("NOT DEFINED FREQ\n");
	}
}

static u32 ddr3_init_main(void)
{
	u32 target_freq;
	u32 reg = 0;
	u32 cpu_freq, fab_opt, hclk_time_ps, soc_num;
	__maybe_unused u32 ecc = DRAM_ECC;
	__maybe_unused int dqs_clk_aligned = 0;
	__maybe_unused u32 scrub_offs, scrub_size;
	__maybe_unused u32 ddr_width = BUS_WIDTH;
	__maybe_unused int status;
	__maybe_unused u32 win_backup[16];
	__maybe_unused struct udevice *udev;
	__maybe_unused int ret;

	/* SoC/Board special Initializtions */
	fab_opt = ddr3_get_fab_opt();

#ifdef CONFIG_SPD_EEPROM
	ret = i2c_get_chip_for_busnum(0, BUS_WIDTH_ECC_TWSI_ADDR, 1, &udev);
	if (ret) {
		printf("Cannot find SPD EEPROM\n");
		return MV_DDR3_TRAINING_ERR_BAD_DIMM_SETUP;
	}
#endif

	ddr3_print_version();
	DEBUG_INIT_S("4\n");
	/* Lib version 5.5.4 */

	fab_opt = ddr3_get_fab_opt();

	/* Switching CPU to MRVL ID */
	soc_num = (reg_read(REG_SAMPLE_RESET_HIGH_ADDR) & SAR1_CPU_CORE_MASK) >>
		SAR1_CPU_CORE_OFFSET;
	switch (soc_num) {
	case 0x3:
		reg_bit_set(CPU_CONFIGURATION_REG(3), CPU_MRVL_ID_OFFSET);
		reg_bit_set(CPU_CONFIGURATION_REG(2), CPU_MRVL_ID_OFFSET);
	case 0x1:
		reg_bit_set(CPU_CONFIGURATION_REG(1), CPU_MRVL_ID_OFFSET);
	case 0x0:
		reg_bit_set(CPU_CONFIGURATION_REG(0), CPU_MRVL_ID_OFFSET);
	default:
		break;
	}

	/* Power down deskew PLL */
#if !defined(MV88F672X)
	/* 0x18780 [25] */
	reg = (reg_read(REG_DDRPHY_APLL_CTRL_ADDR) & ~(1 << 25));
	reg_write(REG_DDRPHY_APLL_CTRL_ADDR, reg);
#endif

	/*
	 * Stage 0 - Set board configuration
	 */
	cpu_freq = ddr3_get_cpu_freq();
	if (fab_opt > FAB_OPT)
		fab_opt = FAB_OPT - 1;

	if (ddr3_get_log_level() > 0)
		print_ddr_target_freq(cpu_freq, fab_opt);

#if defined(MV88F672X)
	get_target_freq(cpu_freq, &target_freq, &hclk_time_ps);
#else
	target_freq = cpu_ddr_ratios[fab_opt][cpu_freq];
	hclk_time_ps = cpu_fab_clk_to_hclk[fab_opt][cpu_freq];
#endif
	if ((target_freq == 0) || (hclk_time_ps == 0)) {
		DEBUG_INIT_S("DDR3 Training Sequence - FAILED - Wrong Sample at Reset Configurations\n");
		if (target_freq == 0) {
			DEBUG_INIT_C("target_freq", target_freq, 2);
			DEBUG_INIT_C("fab_opt", fab_opt, 2);
			DEBUG_INIT_C("cpu_freq", cpu_freq, 2);
		} else if (hclk_time_ps == 0) {
			DEBUG_INIT_C("hclk_time_ps", hclk_time_ps, 2);
			DEBUG_INIT_C("fab_opt", fab_opt, 2);
			DEBUG_INIT_C("cpu_freq", cpu_freq, 2);
		}

		return MV_DDR3_TRAINING_ERR_BAD_SAR;
	}

#if defined(ECC_SUPPORT)
	scrub_offs = U_BOOT_START_ADDR;
	scrub_size = U_BOOT_SCRUB_SIZE;
#else
	scrub_offs = 0;
	scrub_size = 0;
#endif

#if defined(ECC_SUPPORT) && defined(AUTO_DETECTION_SUPPORT)
	ecc = 0;
	if (ddr3_check_config(udev, CONFIG_ECC))
		ecc = 1;
#endif

#ifdef DQS_CLK_ALIGNED
	dqs_clk_aligned = 1;
#endif

	/* Check if DRAM is already initialized  */
	if (reg_read(REG_BOOTROM_ROUTINE_ADDR) &
	    (1 << REG_BOOTROM_ROUTINE_DRAM_INIT_OFFS)) {
		DEBUG_INIT_S("DDR3 Training Sequence - 2nd boot - Skip\n");
		return MV_OK;
	}

	/*
	 * Stage 1 - Dunit Setup
	 */

#ifdef DUNIT_STATIC
	/*
	 * For Static D-Unit Setup use must set the correct static values
	 * at the ddr3_*soc*_vars.h file
	 */
	DEBUG_INIT_FULL_S("DDR3 Training Sequence - Static MC Init\n");
	ddr3_static_mc_init();

#ifdef ECC_SUPPORT
	ecc = DRAM_ECC;
	if (ecc) {
		reg = reg_read(REG_SDRAM_CONFIG_ADDR);
		reg |= (1 << REG_SDRAM_CONFIG_ECC_OFFS);
		reg_write(REG_SDRAM_CONFIG_ADDR, reg);
	}
#endif
#endif

#if defined(MV88F78X60) || defined(MV88F672X)
#if defined(AUTO_DETECTION_SUPPORT)
	/*
	 * Configurations for both static and dynamic MC setups
	 *
	 * Dynamically Set 32Bit and ECC for AXP (Relevant only for
	 * Marvell DB boards)
	 */
	if (ddr3_check_config(udev, CONFIG_BUS_WIDTH)) {
		ddr_width = 32;
		DEBUG_INIT_S("DDR3 Training Sequence - DRAM bus width 32Bit\n");
	}
#endif

#if defined(MV88F672X)
	reg = reg_read(REG_SDRAM_CONFIG_ADDR);
	if ((reg >> 15) & 1)
		ddr_width = 32;
	else
		ddr_width = 16;
#endif
#endif

#ifdef DUNIT_SPD
	status = ddr3_dunit_setup(ecc, hclk_time_ps, &ddr_width);
	if (MV_OK != status) {
		DEBUG_INIT_S("DDR3 Training Sequence - FAILED (ddr3 Dunit Setup)\n");
		return status;
	}
#endif

	/* Fix read ready phases for all SOC in reg 0x15C8 */
	reg = reg_read(REG_TRAINING_DEBUG_3_ADDR);
	reg &= ~(REG_TRAINING_DEBUG_3_MASK);
	reg |= 0x4;		/* Phase 0 */
	reg &= ~(REG_TRAINING_DEBUG_3_MASK << REG_TRAINING_DEBUG_3_OFFS);
	reg |= (0x4 << (1 * REG_TRAINING_DEBUG_3_OFFS));	/* Phase 1 */
	reg &= ~(REG_TRAINING_DEBUG_3_MASK << (3 * REG_TRAINING_DEBUG_3_OFFS));
	reg |= (0x6 << (3 * REG_TRAINING_DEBUG_3_OFFS));	/* Phase 3 */
	reg &= ~(REG_TRAINING_DEBUG_3_MASK << (4 * REG_TRAINING_DEBUG_3_OFFS));
	reg |= (0x6 << (4 * REG_TRAINING_DEBUG_3_OFFS));
	reg &= ~(REG_TRAINING_DEBUG_3_MASK << (5 * REG_TRAINING_DEBUG_3_OFFS));
	reg |= (0x6 << (5 * REG_TRAINING_DEBUG_3_OFFS));
	reg_write(REG_TRAINING_DEBUG_3_ADDR, reg);

#if defined(MV88F672X)
	/*
	 * AxiBrespMode[8] = Compliant,
	 * AxiAddrDecodeCntrl[11] = Internal,
	 * AxiDataBusWidth[0] = 128bit
	 */
	/* 0x14A8 - AXI Control Register */
	reg_write(REG_DRAM_AXI_CTRL_ADDR, 0);
#else
	/* 0x14A8 - AXI Control Register */
	reg_write(REG_DRAM_AXI_CTRL_ADDR, 0x00000100);
	reg_write(REG_CDI_CONFIG_ADDR, 0x00000006);

	if ((ddr_width == 64) && (reg_read(REG_DDR_IO_ADDR) &
				  (1 << REG_DDR_IO_CLK_RATIO_OFFS))) {
		/* 0x14A8 - AXI Control Register */
		reg_write(REG_DRAM_AXI_CTRL_ADDR, 0x00000101);
		reg_write(REG_CDI_CONFIG_ADDR, 0x00000007);
	}
#endif

#if !defined(MV88F67XX)
	/*
	 * ARMADA-370 activate DLB later at the u-boot,
	 * Armada38x - No DLB activation at this time
	 */
	reg_write(DLB_BUS_OPTIMIZATION_WEIGHTS_REG, 0x18C01E);

#if defined(MV88F78X60)
	/* WA according to eratta GL-8672902*/
	if (mv_ctrl_rev_get() == MV_78XX0_B0_REV)
		reg_write(DLB_BUS_OPTIMIZATION_WEIGHTS_REG, 0xc19e);
#endif

	reg_write(DLB_AGING_REGISTER, 0x0f7f007f);
	reg_write(DLB_EVICTION_CONTROL_REG, 0x0);
	reg_write(DLB_EVICTION_TIMERS_REGISTER_REG, 0x00FF3C1F);

	reg_write(MBUS_UNITS_PRIORITY_CONTROL_REG, 0x55555555);
	reg_write(FABRIC_UNITS_PRIORITY_CONTROL_REG, 0xAA);
	reg_write(MBUS_UNITS_PREFETCH_CONTROL_REG, 0xffff);
	reg_write(FABRIC_UNITS_PREFETCH_CONTROL_REG, 0xf0f);

#if defined(MV88F78X60)
	/* WA according to eratta GL-8672902 */
	if (mv_ctrl_rev_get() == MV_78XX0_B0_REV) {
		reg = reg_read(REG_STATIC_DRAM_DLB_CONTROL);
		reg |= DLB_ENABLE;
		reg_write(REG_STATIC_DRAM_DLB_CONTROL, reg);
	}
#endif /* end defined(MV88F78X60) */
#endif /* end !defined(MV88F67XX) */

	if (ddr3_get_log_level() >= MV_LOG_LEVEL_1)
		print_dunit_setup();

	/*
	 * Stage 2 - Training Values Setup
	 */
#ifdef STATIC_TRAINING
	/*
	 * DRAM Init - After all the D-unit values are set, its time to init
	 * the D-unit
	 */
	/* Wait for '0' */
	reg_write(REG_SDRAM_INIT_CTRL_ADDR, 0x1);
	do {
		reg = (reg_read(REG_SDRAM_INIT_CTRL_ADDR)) &
			(1 << REG_SDRAM_INIT_CTRL_OFFS);
	} while (reg);

	/* ddr3 init using static parameters - HW training is disabled */
	DEBUG_INIT_FULL_S("DDR3 Training Sequence - Static Training Parameters\n");
	ddr3_static_training_init();

#if defined(MV88F78X60)
	/*
	 * If ECC is enabled, need to scrub the U-Boot area memory region -
	 * Run training function with Xor bypass just to scrub the memory
	 */
	status = ddr3_hw_training(target_freq, ddr_width,
				  1, scrub_offs, scrub_size,
				  dqs_clk_aligned, DDR3_TRAINING_DEBUG,
				  REG_DIMM_SKIP_WL);
	if (MV_OK != status) {
		DEBUG_INIT_FULL_S("DDR3 Training Sequence - FAILED\n");
		return status;
	}
#endif
#else
	/* Set X-BAR windows for the training sequence */
	ddr3_save_and_set_training_windows(win_backup);

	/* Run DDR3 Training Sequence */
	/* DRAM Init */
	reg_write(REG_SDRAM_INIT_CTRL_ADDR, 0x1);
	do {
		reg = (reg_read(REG_SDRAM_INIT_CTRL_ADDR)) &
			(1 << REG_SDRAM_INIT_CTRL_OFFS);
	} while (reg);		/* Wait for '0' */

	/* ddr3 init using DDR3 HW training procedure */
	DEBUG_INIT_FULL_S("DDR3 Training Sequence - HW Training Procedure\n");
	status = ddr3_hw_training(target_freq, ddr_width,
				  0, scrub_offs, scrub_size,
				  dqs_clk_aligned, DDR3_TRAINING_DEBUG,
				  REG_DIMM_SKIP_WL);
	if (MV_OK != status) {
		DEBUG_INIT_FULL_S("DDR3 Training Sequence - FAILED\n");
		return status;
	}
#endif

	/*
	 * Stage 3 - Finish
	 */
#if defined(MV88F78X60) || defined(MV88F672X)
	/* Disable ECC Ignore bit */
	reg = reg_read(REG_SDRAM_CONFIG_ADDR) &
		~(1 << REG_SDRAM_CONFIG_IERR_OFFS);
	reg_write(REG_SDRAM_CONFIG_ADDR, reg);
#endif

#if !defined(STATIC_TRAINING)
	/* Restore and set windows */
	ddr3_restore_and_set_final_windows(win_backup);
#endif

	/* Update DRAM init indication in bootROM register */
	reg = reg_read(REG_BOOTROM_ROUTINE_ADDR);
	reg_write(REG_BOOTROM_ROUTINE_ADDR,
		  reg | (1 << REG_BOOTROM_ROUTINE_DRAM_INIT_OFFS));

#if !defined(MV88F67XX)
#if defined(MV88F78X60)
	if (mv_ctrl_rev_get() == MV_78XX0_B0_REV) {
		reg = reg_read(REG_SDRAM_CONFIG_ADDR);
		if (ecc == 0)
			reg_write(REG_SDRAM_CONFIG_ADDR, reg | (1 << 19));
	}
#endif /* end defined(MV88F78X60) */

	reg_write(DLB_EVICTION_CONTROL_REG, 0x9);

	reg = reg_read(REG_STATIC_DRAM_DLB_CONTROL);
	reg |= (DLB_ENABLE | DLB_WRITE_COALESING | DLB_AXI_PREFETCH_EN |
		DLB_MBUS_PREFETCH_EN | PREFETCH_NLNSZTR);
	reg_write(REG_STATIC_DRAM_DLB_CONTROL, reg);
#endif /* end !defined(MV88F67XX) */

#ifdef STATIC_TRAINING
	DEBUG_INIT_S("DDR3 Training Sequence - Ended Successfully (S)\n");
#else
	DEBUG_INIT_S("DDR3 Training Sequence - Ended Successfully\n");
#endif

	return MV_OK;
}

/*
 * Name:     ddr3_get_cpu_freq
 * Desc:     read S@R and return CPU frequency
 * Args:
 * Notes:
 * Returns:  required value
 */

u32 ddr3_get_cpu_freq(void)
{
	u32 reg, cpu_freq;

#if defined(MV88F672X)
	/* Read sample at reset setting */
	reg = reg_read(REG_SAMPLE_RESET_HIGH_ADDR);	/* 0xE8200 */
	cpu_freq = (reg & REG_SAMPLE_RESET_CPU_FREQ_MASK) >>
		REG_SAMPLE_RESET_CPU_FREQ_OFFS;
#else
	/* Read sample at reset setting */
	reg = reg_read(REG_SAMPLE_RESET_LOW_ADDR);	/* 0x18230 [23:21] */
#if defined(MV88F78X60)
	cpu_freq = (reg & REG_SAMPLE_RESET_CPU_FREQ_MASK) >>
		REG_SAMPLE_RESET_CPU_FREQ_OFFS;
	reg = reg_read(REG_SAMPLE_RESET_HIGH_ADDR);	/* 0x18234 [20] */
	cpu_freq |= (((reg >> REG_SAMPLE_RESET_HIGH_CPU_FREQ_OFFS) & 0x1) << 3);
#elif defined(MV88F67XX)
	cpu_freq = (reg & REG_SAMPLE_RESET_CPU_FREQ_MASK) >>
		REG_SAMPLE_RESET_CPU_FREQ_OFFS;
#endif
#endif

	return cpu_freq;
}

/*
 * Name:     ddr3_get_fab_opt
 * Desc:     read S@R and return CPU frequency
 * Args:
 * Notes:
 * Returns:  required value
 */
u32 ddr3_get_fab_opt(void)
{
	__maybe_unused u32 reg, fab_opt;

#if defined(MV88F672X)
	return 0;		/* No fabric */
#else
	/* Read sample at reset setting */
	reg = reg_read(REG_SAMPLE_RESET_LOW_ADDR);
	fab_opt = (reg & REG_SAMPLE_RESET_FAB_MASK) >>
		REG_SAMPLE_RESET_FAB_OFFS;

#if defined(MV88F78X60)
	reg = reg_read(REG_SAMPLE_RESET_HIGH_ADDR);
	fab_opt |= (((reg >> 19) & 0x1) << 4);
#endif

	return fab_opt;
#endif
}

/*
 * Name:     ddr3_get_vco_freq
 * Desc:     read S@R and return VCO frequency
 * Args:
 * Notes:
 * Returns:  required value
 */
u32 ddr3_get_vco_freq(void)
{
	u32 fab, cpu_freq, ui_vco_freq;

	fab = ddr3_get_fab_opt();
	cpu_freq = ddr3_get_cpu_freq();

	if (fab == 2 || fab == 3 || fab == 7 || fab == 8 || fab == 10 ||
	    fab == 15 || fab == 17 || fab == 20)
		ui_vco_freq = cpu_freq + CLK_CPU;
	else
		ui_vco_freq = cpu_freq;

	return ui_vco_freq;
}

#ifdef STATIC_TRAINING
/*
 * Name:     ddr3_static_training_init - Init DDR3 Training with
 *           static parameters
 * Desc:     Use this routine to init the controller without the HW training
 *           procedure
 *           User must provide compatible header file with registers data.
 * Args:     None.
 * Notes:
 * Returns:  None.
 */
void ddr3_static_training_init(void)
{
	MV_DRAM_MODES *ddr_mode;
	u32 reg;
	int j;

	ddr_mode = ddr3_get_static_ddr_mode();

	j = 0;
	while (ddr_mode->vals[j].reg_addr != 0) {
		udelay(10);	/* haim want to delay each write */
		reg_write(ddr_mode->vals[j].reg_addr,
			  ddr_mode->vals[j].reg_value);

		if (ddr_mode->vals[j].reg_addr ==
		    REG_PHY_REGISTRY_FILE_ACCESS_ADDR)
			do {
				reg = reg_read(REG_PHY_REGISTRY_FILE_ACCESS_ADDR) &
					REG_PHY_REGISTRY_FILE_ACCESS_OP_DONE;
			} while (reg);
		j++;
	}
}
#endif

/*
 * Name:     ddr3_get_static_mc_value - Init Memory controller with static
 *           parameters
 * Desc:     Use this routine to init the controller without the HW training
 *           procedure
 *           User must provide compatible header file with registers data.
 * Args:     None.
 * Notes:
 * Returns:  None.
 */
u32 ddr3_get_static_mc_value(u32 reg_addr, u32 offset1, u32 mask1, u32 offset2,
			     u32 mask2)
{
	u32 reg, tmp;

	reg = reg_read(reg_addr);

	tmp = (reg >> offset1) & mask1;
	if (mask2)
		tmp |= (reg >> offset2) & mask2;

	return tmp;
}

/*
 * Name:     ddr3_get_static_ddr_mode - Init Memory controller with static
 *           parameters
 * Desc:     Use this routine to init the controller without the HW training
 *           procedure
 *           User must provide compatible header file with registers data.
 * Args:     None.
 * Notes:
 * Returns:  None.
 */
__weak MV_DRAM_MODES *ddr3_get_static_ddr_mode(void)
{
	u32 chip_board_rev, i;
	u32 size;

	/* Do not modify this code. relevant only for marvell Boards */
#if defined(DB_78X60_PCAC)
	chip_board_rev = Z1_PCAC;
#elif defined(DB_78X60_AMC)
	chip_board_rev = A0_AMC;
#elif defined(DB_88F6710_PCAC)
	chip_board_rev = A0_PCAC;
#elif defined(RD_88F6710)
	chip_board_rev = A0_RD;
#elif defined(MV88F672X)
	chip_board_rev = mv_board_id_get();
#else
	chip_board_rev = A0;
#endif

	size = sizeof(ddr_modes) / sizeof(MV_DRAM_MODES);
	for (i = 0; i < size; i++) {
		if ((ddr3_get_cpu_freq() == ddr_modes[i].cpu_freq) &&
		    (ddr3_get_fab_opt() == ddr_modes[i].fab_freq) &&
		    (chip_board_rev == ddr_modes[i].chip_board_rev))
			return &ddr_modes[i];
	}

	return &ddr_modes[0];
}

#ifdef DUNIT_STATIC
/*
 * Name:     ddr3_static_mc_init - Init Memory controller with static parameters
 * Desc:     Use this routine to init the controller without the HW training
 *           procedure
 *           User must provide compatible header file with registers data.
 * Args:     None.
 * Notes:
 * Returns:  None.
 */
void ddr3_static_mc_init(void)
{
	MV_DRAM_MODES *ddr_mode;
	u32 reg;
	int j;

	ddr_mode = ddr3_get_static_ddr_mode();
	j = 0;
	while (ddr_mode->regs[j].reg_addr != 0) {
		reg_write(ddr_mode->regs[j].reg_addr,
			  ddr_mode->regs[j].reg_value);
		if (ddr_mode->regs[j].reg_addr ==
		    REG_PHY_REGISTRY_FILE_ACCESS_ADDR)
			do {
				reg = reg_read(REG_PHY_REGISTRY_FILE_ACCESS_ADDR) &
					REG_PHY_REGISTRY_FILE_ACCESS_OP_DONE;
			} while (reg);
		j++;
	}
}
#endif

/*
 * Name:     ddr3_check_config - Check user configurations: ECC/MultiCS
 * Desc:
 * Args:     twsi Address
 * Notes:    Only Available for ArmadaXP/Armada 370 DB boards
 * Returns:  None.
 */
int ddr3_check_config(struct udevice *udev, MV_CONFIG_TYPE config_type)
{
#ifdef AUTO_DETECTION_SUPPORT
	u8 data = 0;
	int ret;
	int offset;

	if ((config_type == CONFIG_ECC) || (config_type == CONFIG_BUS_WIDTH))
		offset = 1;
	else
		offset = 0;

	ret = dm_i2c_read(udev, offset, &data, 1);
	if (!ret) {
		switch (config_type) {
		case CONFIG_ECC:
			if (data & 0x2)
				return 1;
			break;
		case CONFIG_BUS_WIDTH:
			if (data & 0x1)
				return 1;
			break;
#ifdef DB_88F6710
		case CONFIG_MULTI_CS:
			if (CFG_MULTI_CS_MODE(data))
				return 1;
			break;
#else
		case CONFIG_MULTI_CS:
			break;
#endif
		}
	}
#endif

	return 0;
}

/*
 * Name:     ddr3_cl_to_valid_cl - this return register matching CL value
 * Desc:
 * Args:     clValue - the value

 * Notes:
 * Returns:  required CL value
 */
u32 ddr3_cl_to_valid_cl(u32 cl)
{
	switch (cl) {
	case 5:
		return 2;
		break;
	case 6:
		return 4;
		break;
	case 7:
		return 6;
		break;
	case 8:
		return 8;
		break;
	case 9:
		return 10;
		break;
	case 10:
		return 12;
		break;
	case 11:
		return 14;
		break;
	case 12:
		return 1;
		break;
	case 13:
		return 3;
		break;
	case 14:
		return 5;
		break;
	default:
		return 2;
	}
}

/*
 * Name:     ddr3_cl_to_valid_cl - this return register matching CL value
 * Desc:
 * Args:     clValue - the value
 * Notes:
 * Returns:  required CL value
 */
u32 ddr3_valid_cl_to_cl(u32 ui_valid_cl)
{
	switch (ui_valid_cl) {
	case 1:
		return 12;
		break;
	case 2:
		return 5;
		break;
	case 3:
		return 13;
		break;
	case 4:
		return 6;
		break;
	case 5:
		return 14;
		break;
	case 6:
		return 7;
		break;
	case 8:
		return 8;
		break;
	case 10:
		return 9;
		break;
	case 12:
		return 10;
		break;
	case 14:
		return 11;
		break;
	default:
		return 0;
	}
}

/*
 * Name:     ddr3_get_cs_num_from_reg
 * Desc:
 * Args:
 * Notes:
 * Returns:
 */
u32 ddr3_get_cs_num_from_reg(void)
{
	u32 cs_ena = ddr3_get_cs_ena_from_reg();
	u32 cs_count = 0;
	u32 cs;

	for (cs = 0; cs < MAX_CS; cs++) {
		if (cs_ena & (1 << cs))
			cs_count++;
	}

	return cs_count;
}

/*
 * Name:     ddr3_get_cs_ena_from_reg
 * Desc:
 * Args:
 * Notes:
 * Returns:
 */
u32 ddr3_get_cs_ena_from_reg(void)
{
	return reg_read(REG_DDR3_RANK_CTRL_ADDR) &
		REG_DDR3_RANK_CTRL_CS_ENA_MASK;
}

/*
 * mv_ctrl_rev_get - Get Marvell controller device revision number
 *
 * DESCRIPTION:
 *       This function returns 8bit describing the device revision as defined
 *       in PCI Express Class Code and Revision ID Register.
 *
 * INPUT:
 *       None.
 *
 * OUTPUT:
 *       None.
 *
 * RETURN:
 *       8bit desscribing Marvell controller revision number
 *
 */
#if !defined(MV88F672X)
u8 mv_ctrl_rev_get(void)
{
	u8 rev_num;

#if defined(MV_INCLUDE_CLK_PWR_CNTRL)
	/* Check pex power state */
	u32 pex_power;
	pex_power = mv_ctrl_pwr_clck_get(PEX_UNIT_ID, 0);
	if (pex_power == 0)
		mv_ctrl_pwr_clck_set(PEX_UNIT_ID, 0, 1);
#endif
	rev_num = (u8)reg_read(PEX_CFG_DIRECT_ACCESS(0,
			PCI_CLASS_CODE_AND_REVISION_ID));

#if defined(MV_INCLUDE_CLK_PWR_CNTRL)
	/* Return to power off state */
	if (pex_power == 0)
		mv_ctrl_pwr_clck_set(PEX_UNIT_ID, 0, 0);
#endif

	return (rev_num & PCCRIR_REVID_MASK) >> PCCRIR_REVID_OFFS;
}

#endif

#if defined(MV88F672X)
void get_target_freq(u32 freq_mode, u32 *ddr_freq, u32 *hclk_ps)
{
	u32 tmp, hclk;

	switch (freq_mode) {
	case CPU_333MHz_DDR_167MHz_L2_167MHz:
		hclk = 84;
		tmp = DDR_100;
		break;
	case CPU_266MHz_DDR_266MHz_L2_133MHz:
	case CPU_333MHz_DDR_222MHz_L2_167MHz:
	case CPU_400MHz_DDR_200MHz_L2_200MHz:
	case CPU_400MHz_DDR_267MHz_L2_200MHz:
	case CPU_533MHz_DDR_267MHz_L2_267MHz:
	case CPU_500MHz_DDR_250MHz_L2_250MHz:
	case CPU_600MHz_DDR_300MHz_L2_300MHz:
	case CPU_800MHz_DDR_267MHz_L2_400MHz:
	case CPU_900MHz_DDR_300MHz_L2_450MHz:
		tmp = DDR_300;
		hclk = 150;
		break;
	case CPU_333MHz_DDR_333MHz_L2_167MHz:
	case CPU_500MHz_DDR_334MHz_L2_250MHz:
	case CPU_666MHz_DDR_333MHz_L2_333MHz:
		tmp = DDR_333;
		hclk = 165;
		break;
	case CPU_533MHz_DDR_356MHz_L2_267MHz:
		tmp = DDR_360;
		hclk = 180;
		break;
	case CPU_400MHz_DDR_400MHz_L2_200MHz:
	case CPU_600MHz_DDR_400MHz_L2_300MHz:
	case CPU_800MHz_DDR_400MHz_L2_400MHz:
	case CPU_400MHz_DDR_400MHz_L2_400MHz:
		tmp = DDR_400;
		hclk = 200;
		break;
	case CPU_666MHz_DDR_444MHz_L2_333MHz:
	case CPU_900MHz_DDR_450MHz_L2_450MHz:
		tmp = DDR_444;
		hclk = 222;
		break;
	case CPU_500MHz_DDR_500MHz_L2_250MHz:
	case CPU_1000MHz_DDR_500MHz_L2_500MHz:
	case CPU_1000MHz_DDR_500MHz_L2_333MHz:
		tmp = DDR_500;
		hclk = 250;
		break;
	case CPU_533MHz_DDR_533MHz_L2_267MHz:
	case CPU_800MHz_DDR_534MHz_L2_400MHz:
	case CPU_1100MHz_DDR_550MHz_L2_550MHz:
		tmp = DDR_533;
		hclk = 267;
		break;
	case CPU_600MHz_DDR_600MHz_L2_300MHz:
	case CPU_900MHz_DDR_600MHz_L2_450MHz:
	case CPU_1200MHz_DDR_600MHz_L2_600MHz:
		tmp = DDR_600;
		hclk = 300;
		break;
	case CPU_666MHz_DDR_666MHz_L2_333MHz:
	case CPU_1000MHz_DDR_667MHz_L2_500MHz:
		tmp = DDR_666;
		hclk = 333;
		break;
	default:
		*ddr_freq = 0;
		*hclk_ps = 0;
		break;
	}

	*ddr_freq = tmp;		/* DDR freq define */
	*hclk_ps = 1000000 / hclk;	/* values are 1/HCLK in ps */

	return;
}
#endif
