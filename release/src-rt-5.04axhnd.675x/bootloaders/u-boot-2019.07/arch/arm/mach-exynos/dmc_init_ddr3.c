// SPDX-License-Identifier: GPL-2.0+
/*
 * DDR3 mem setup file for board based on EXYNOS5
 *
 * Copyright (C) 2012 Samsung Electronics
 */

#include <common.h>
#include <config.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/cpu.h>
#include <asm/arch/dmc.h>
#include <asm/arch/power.h>
#include "common_setup.h"
#include "exynos5_setup.h"
#include "clock_init.h"

#define TIMEOUT_US		10000
#define NUM_BYTE_LANES		4
#define DEFAULT_DQS		8
#define DEFAULT_DQS_X4		((DEFAULT_DQS << 24) || (DEFAULT_DQS << 16) \
				|| (DEFAULT_DQS << 8) || (DEFAULT_DQS << 0))

#ifdef CONFIG_EXYNOS5250
static void reset_phy_ctrl(void)
{
	struct exynos5_clock *clk =
		(struct exynos5_clock *)samsung_get_base_clock();

	writel(DDR3PHY_CTRL_PHY_RESET_OFF, &clk->lpddr3phy_ctrl);
	writel(DDR3PHY_CTRL_PHY_RESET, &clk->lpddr3phy_ctrl);
}

int ddr3_mem_ctrl_init(struct mem_timings *mem, int reset)
{
	unsigned int val;
	struct exynos5_phy_control *phy0_ctrl, *phy1_ctrl;
	struct exynos5_dmc *dmc;
	int i;

	phy0_ctrl = (struct exynos5_phy_control *)samsung_get_base_dmc_phy();
	phy1_ctrl = (struct exynos5_phy_control *)(samsung_get_base_dmc_phy()
							+ DMC_OFFSET);
	dmc = (struct exynos5_dmc *)samsung_get_base_dmc_ctrl();

	if (reset)
		reset_phy_ctrl();

	/* Set Impedance Output Driver */
	val = (mem->impedance << CA_CK_DRVR_DS_OFFSET) |
		(mem->impedance << CA_CKE_DRVR_DS_OFFSET) |
		(mem->impedance << CA_CS_DRVR_DS_OFFSET) |
		(mem->impedance << CA_ADR_DRVR_DS_OFFSET);
	writel(val, &phy0_ctrl->phy_con39);
	writel(val, &phy1_ctrl->phy_con39);

	/* Set Read Latency and Burst Length for PHY0 and PHY1 */
	val = (mem->ctrl_bstlen << PHY_CON42_CTRL_BSTLEN_SHIFT) |
		(mem->ctrl_rdlat << PHY_CON42_CTRL_RDLAT_SHIFT);
	writel(val, &phy0_ctrl->phy_con42);
	writel(val, &phy1_ctrl->phy_con42);

	/* ZQ Calibration */
	if (dmc_config_zq(mem, &phy0_ctrl->phy_con16, &phy1_ctrl->phy_con16,
			  &phy0_ctrl->phy_con17, &phy1_ctrl->phy_con17))
		return SETUP_ERR_ZQ_CALIBRATION_FAILURE;

	/* DQ Signal */
	writel(mem->phy0_pulld_dqs, &phy0_ctrl->phy_con14);
	writel(mem->phy1_pulld_dqs, &phy1_ctrl->phy_con14);

	writel(mem->concontrol | (mem->rd_fetch << CONCONTROL_RD_FETCH_SHIFT)
		| (mem->dfi_init_start << CONCONTROL_DFI_INIT_START_SHIFT),
		&dmc->concontrol);

	update_reset_dll(&dmc->phycontrol0, DDR_MODE_DDR3);

	/* DQS Signal */
	writel(mem->phy0_dqs, &phy0_ctrl->phy_con4);
	writel(mem->phy1_dqs, &phy1_ctrl->phy_con4);

	writel(mem->phy0_dq, &phy0_ctrl->phy_con6);
	writel(mem->phy1_dq, &phy1_ctrl->phy_con6);

	writel(mem->phy0_tFS, &phy0_ctrl->phy_con10);
	writel(mem->phy1_tFS, &phy1_ctrl->phy_con10);

	val = (mem->ctrl_start_point << PHY_CON12_CTRL_START_POINT_SHIFT) |
		(mem->ctrl_inc << PHY_CON12_CTRL_INC_SHIFT) |
		(mem->ctrl_dll_on << PHY_CON12_CTRL_DLL_ON_SHIFT) |
		(mem->ctrl_ref << PHY_CON12_CTRL_REF_SHIFT);
	writel(val, &phy0_ctrl->phy_con12);
	writel(val, &phy1_ctrl->phy_con12);

	/* Start DLL locking */
	writel(val | (mem->ctrl_start << PHY_CON12_CTRL_START_SHIFT),
	       &phy0_ctrl->phy_con12);
	writel(val | (mem->ctrl_start << PHY_CON12_CTRL_START_SHIFT),
	       &phy1_ctrl->phy_con12);

	update_reset_dll(&dmc->phycontrol0, DDR_MODE_DDR3);

	writel(mem->concontrol | (mem->rd_fetch << CONCONTROL_RD_FETCH_SHIFT),
	       &dmc->concontrol);

	/* Memory Channel Inteleaving Size */
	writel(mem->iv_size, &dmc->ivcontrol);

	writel(mem->memconfig, &dmc->memconfig0);
	writel(mem->memconfig, &dmc->memconfig1);
	writel(mem->membaseconfig0, &dmc->membaseconfig0);
	writel(mem->membaseconfig1, &dmc->membaseconfig1);

	/* Precharge Configuration */
	writel(mem->prechconfig_tp_cnt << PRECHCONFIG_TP_CNT_SHIFT,
	       &dmc->prechconfig);

	/* Power Down mode Configuration */
	writel(mem->dpwrdn_cyc << PWRDNCONFIG_DPWRDN_CYC_SHIFT |
		mem->dsref_cyc << PWRDNCONFIG_DSREF_CYC_SHIFT,
		&dmc->pwrdnconfig);

	/* TimingRow, TimingData, TimingPower and Timingaref
	 * values as per Memory AC parameters
	 */
	writel(mem->timing_ref, &dmc->timingref);
	writel(mem->timing_row, &dmc->timingrow);
	writel(mem->timing_data, &dmc->timingdata);
	writel(mem->timing_power, &dmc->timingpower);

	/* Send PALL command */
	dmc_config_prech(mem, &dmc->directcmd);

	/* Send NOP, MRS and ZQINIT commands */
	dmc_config_mrs(mem, &dmc->directcmd);

	if (mem->gate_leveling_enable) {
		val = PHY_CON0_RESET_VAL;
		val |= P0_CMD_EN;
		writel(val, &phy0_ctrl->phy_con0);
		writel(val, &phy1_ctrl->phy_con0);

		val = PHY_CON2_RESET_VAL;
		val |= INIT_DESKEW_EN;
		writel(val, &phy0_ctrl->phy_con2);
		writel(val, &phy1_ctrl->phy_con2);

		val = PHY_CON0_RESET_VAL;
		val |= P0_CMD_EN;
		val |= BYTE_RDLVL_EN;
		writel(val, &phy0_ctrl->phy_con0);
		writel(val, &phy1_ctrl->phy_con0);

		val = (mem->ctrl_start_point <<
				PHY_CON12_CTRL_START_POINT_SHIFT) |
			(mem->ctrl_inc << PHY_CON12_CTRL_INC_SHIFT) |
			(mem->ctrl_force << PHY_CON12_CTRL_FORCE_SHIFT) |
			(mem->ctrl_start << PHY_CON12_CTRL_START_SHIFT) |
			(mem->ctrl_ref << PHY_CON12_CTRL_REF_SHIFT);
		writel(val, &phy0_ctrl->phy_con12);
		writel(val, &phy1_ctrl->phy_con12);

		val = PHY_CON2_RESET_VAL;
		val |= INIT_DESKEW_EN;
		val |= RDLVL_GATE_EN;
		writel(val, &phy0_ctrl->phy_con2);
		writel(val, &phy1_ctrl->phy_con2);

		val = PHY_CON0_RESET_VAL;
		val |= P0_CMD_EN;
		val |= BYTE_RDLVL_EN;
		val |= CTRL_SHGATE;
		writel(val, &phy0_ctrl->phy_con0);
		writel(val, &phy1_ctrl->phy_con0);

		val = PHY_CON1_RESET_VAL;
		val &= ~(CTRL_GATEDURADJ_MASK);
		writel(val, &phy0_ctrl->phy_con1);
		writel(val, &phy1_ctrl->phy_con1);

		writel(CTRL_RDLVL_GATE_ENABLE, &dmc->rdlvl_config);
		i = TIMEOUT_US;
		while ((readl(&dmc->phystatus) &
			(RDLVL_COMPLETE_CHO | RDLVL_COMPLETE_CH1)) !=
			(RDLVL_COMPLETE_CHO | RDLVL_COMPLETE_CH1) && i > 0) {
			/*
			 * TODO(waihong): Comment on how long this take to
			 * timeout
			 */
			sdelay(100);
			i--;
		}
		if (!i)
			return SETUP_ERR_RDLV_COMPLETE_TIMEOUT;
		writel(CTRL_RDLVL_GATE_DISABLE, &dmc->rdlvl_config);

		writel(0, &phy0_ctrl->phy_con14);
		writel(0, &phy1_ctrl->phy_con14);

		val = (mem->ctrl_start_point <<
				PHY_CON12_CTRL_START_POINT_SHIFT) |
			(mem->ctrl_inc << PHY_CON12_CTRL_INC_SHIFT) |
			(mem->ctrl_force << PHY_CON12_CTRL_FORCE_SHIFT) |
			(mem->ctrl_start << PHY_CON12_CTRL_START_SHIFT) |
			(mem->ctrl_dll_on << PHY_CON12_CTRL_DLL_ON_SHIFT) |
			(mem->ctrl_ref << PHY_CON12_CTRL_REF_SHIFT);
		writel(val, &phy0_ctrl->phy_con12);
		writel(val, &phy1_ctrl->phy_con12);

		update_reset_dll(&dmc->phycontrol0, DDR_MODE_DDR3);
	}

	/* Send PALL command */
	dmc_config_prech(mem, &dmc->directcmd);

	writel(mem->memcontrol, &dmc->memcontrol);

	/* Set DMC Concontrol and enable auto-refresh counter */
	writel(mem->concontrol | (mem->rd_fetch << CONCONTROL_RD_FETCH_SHIFT)
		| (mem->aref_en << CONCONTROL_AREF_EN_SHIFT), &dmc->concontrol);
	return 0;
}
#endif

#ifdef CONFIG_EXYNOS5420
/**
 * RAM address to use in the test.
 *
 * We'll use 4 words at this address and 4 at this address + 0x80 (Ares
 * interleaves channels every 128 bytes).  This will allow us to evaluate all of
 * the chips in a 1 chip per channel (2GB) system and half the chips in a 2
 * chip per channel (4GB) system.  We can't test the 2nd chip since we need to
 * do tests before the 2nd chip is enabled.  Looking at the 2nd chip isn't
 * critical because the 1st and 2nd chip have very similar timings (they'd
 * better have similar timings, since there's only a single adjustment that is
 * shared by both chips).
 */
const unsigned int test_addr = CONFIG_SYS_SDRAM_BASE;

/* Test pattern with which RAM will be tested */
static const unsigned int test_pattern[] = {
	0x5a5a5a5a,
	0xa5a5a5a5,
	0xf0f0f0f0,
	0x0f0f0f0f,
};

/**
 * This function is a test vector for sw read leveling,
 * it compares the read data with the written data.
 *
 * @param ch			DMC channel number
 * @param byte_lane		which DQS byte offset,
 *				possible values are 0,1,2,3
 * @return			TRUE if memory was good, FALSE if not.
 */
static bool dmc_valid_window_test_vector(int ch, int byte_lane)
{
	unsigned int read_data;
	unsigned int mask;
	int i;

	mask = 0xFF << (8 * byte_lane);

	for (i = 0; i < ARRAY_SIZE(test_pattern); i++) {
		read_data = readl(test_addr + i * 4 + ch * 0x80);
		if ((read_data & mask) != (test_pattern[i] & mask))
			return false;
	}

	return true;
}

/**
 * This function returns current read offset value.
 *
 * @param phy_ctrl	pointer to the current phy controller
 */
static unsigned int dmc_get_read_offset_value(struct exynos5420_phy_control
					       *phy_ctrl)
{
	return readl(&phy_ctrl->phy_con4);
}

/**
 * This function performs resync, so that slave DLL is updated.
 *
 * @param phy_ctrl	pointer to the current phy controller
 */
static void ddr_phy_set_do_resync(struct exynos5420_phy_control *phy_ctrl)
{
	setbits_le32(&phy_ctrl->phy_con10, PHY_CON10_CTRL_OFFSETR3);
	clrbits_le32(&phy_ctrl->phy_con10, PHY_CON10_CTRL_OFFSETR3);
}

/**
 * This function sets read offset value register with 'offset'.
 *
 * ...we also call call ddr_phy_set_do_resync().
 *
 * @param phy_ctrl	pointer to the current phy controller
 * @param offset	offset to read DQS
 */
static void dmc_set_read_offset_value(struct exynos5420_phy_control *phy_ctrl,
				      unsigned int offset)
{
	writel(offset, &phy_ctrl->phy_con4);
	ddr_phy_set_do_resync(phy_ctrl);
}

/**
 * Convert a 2s complement byte to a byte with a sign bit.
 *
 * NOTE: you shouldn't use normal math on the number returned by this function.
 *   As an example, -10 = 0xf6.  After this function -10 = 0x8a.  If you wanted
 *   to do math and get the average of 10 and -10 (should be 0):
 *     0x8a + 0xa = 0x94 (-108)
 *     0x94 / 2   = 0xca (-54)
 *   ...and 0xca = sign bit plus 0x4a, or -74
 *
 * Also note that you lose the ability to represent -128 since there are two
 * representations of 0.
 *
 * @param b	The byte to convert in two's complement.
 * @return	The 7-bit value + sign bit.
 */

unsigned char make_signed_byte(signed char b)
{
	if (b < 0)
		return 0x80 | -b;
	else
		return b;
}

/**
 * Test various shifts starting at 'start' and going to 'end'.
 *
 * For each byte lane, we'll walk through shift starting at 'start' and going
 * to 'end' (inclusive).  When we are finally able to read the test pattern
 * we'll store the value in the results array.
 *
 * @param phy_ctrl		pointer to the current phy controller
 * @param ch			channel number
 * @param start			the start shift.  -127 to 127
 * @param end			the end shift.  -127 to 127
 * @param results		we'll store results for each byte lane.
 */

void test_shifts(struct exynos5420_phy_control *phy_ctrl, int ch,
		 int start, int end, int results[NUM_BYTE_LANES])
{
	int incr = (start < end) ? 1 : -1;
	int byte_lane;

	for (byte_lane = 0; byte_lane < NUM_BYTE_LANES; byte_lane++) {
		int shift;

		dmc_set_read_offset_value(phy_ctrl, DEFAULT_DQS_X4);
		results[byte_lane] = DEFAULT_DQS;

		for (shift = start; shift != (end + incr); shift += incr) {
			unsigned int byte_offsetr;
			unsigned int offsetr;

			byte_offsetr = make_signed_byte(shift);

			offsetr = dmc_get_read_offset_value(phy_ctrl);
			offsetr &= ~(0xFF << (8 * byte_lane));
			offsetr |= (byte_offsetr << (8 * byte_lane));
			dmc_set_read_offset_value(phy_ctrl, offsetr);

			if (dmc_valid_window_test_vector(ch, byte_lane)) {
				results[byte_lane] = shift;
				break;
			}
		}
	}
}

/**
 * This function performs SW read leveling to compensate DQ-DQS skew at
 * receiver it first finds the optimal read offset value on each DQS
 * then applies the value to PHY.
 *
 * Read offset value has its min margin and max margin. If read offset
 * value exceeds its min or max margin, read data will have corruption.
 * To avoid this we are doing sw read leveling.
 *
 * SW read leveling is:
 * 1> Finding offset value's left_limit and right_limit
 * 2> and calculate its center value
 * 3> finally programs that center value to PHY
 * 4> then PHY gets its optimal offset value.
 *
 * @param phy_ctrl		pointer to the current phy controller
 * @param ch			channel number
 * @param coarse_lock_val	The coarse lock value read from PHY_CON13.
 *				(0 - 0x7f)
 */
static void software_find_read_offset(struct exynos5420_phy_control *phy_ctrl,
				      int ch, unsigned int coarse_lock_val)
{
	unsigned int offsetr_cent;
	int byte_lane;
	int left_limit;
	int right_limit;
	int left[NUM_BYTE_LANES];
	int right[NUM_BYTE_LANES];
	int i;

	/* Fill the memory with test patterns */
	for (i = 0; i < ARRAY_SIZE(test_pattern); i++)
		writel(test_pattern[i], test_addr + i * 4 + ch * 0x80);

	/* Figure out the limits we'll test with; keep -127 < limit < 127 */
	left_limit = DEFAULT_DQS - coarse_lock_val;
	right_limit = DEFAULT_DQS + coarse_lock_val;
	if (right_limit > 127)
		right_limit = 127;

	/* Fill in the location where reads were OK from left and right */
	test_shifts(phy_ctrl, ch, left_limit, right_limit, left);
	test_shifts(phy_ctrl, ch, right_limit, left_limit, right);

	/* Make a final value by taking the center between the left and right */
	offsetr_cent = 0;
	for (byte_lane = 0; byte_lane < NUM_BYTE_LANES; byte_lane++) {
		int temp_center;
		unsigned int vmwc;

		temp_center = (left[byte_lane] + right[byte_lane]) / 2;
		vmwc = make_signed_byte(temp_center);
		offsetr_cent |= vmwc << (8 * byte_lane);
	}
	dmc_set_read_offset_value(phy_ctrl, offsetr_cent);
}

int ddr3_mem_ctrl_init(struct mem_timings *mem, int reset)
{
	struct exynos5420_clock *clk =
		(struct exynos5420_clock *)samsung_get_base_clock();
	struct exynos5420_power *power =
		(struct exynos5420_power *)samsung_get_base_power();
	struct exynos5420_phy_control *phy0_ctrl, *phy1_ctrl;
	struct exynos5420_dmc *drex0, *drex1;
	struct exynos5420_tzasc *tzasc0, *tzasc1;
	struct exynos5_power *pmu;
	uint32_t val, n_lock_r, n_lock_w_phy0, n_lock_w_phy1;
	uint32_t lock0_info, lock1_info;
	int chip;
	int i;

	phy0_ctrl = (struct exynos5420_phy_control *)samsung_get_base_dmc_phy();
	phy1_ctrl = (struct exynos5420_phy_control *)(samsung_get_base_dmc_phy()
							+ DMC_OFFSET);
	drex0 = (struct exynos5420_dmc *)samsung_get_base_dmc_ctrl();
	drex1 = (struct exynos5420_dmc *)(samsung_get_base_dmc_ctrl()
							+ DMC_OFFSET);
	tzasc0 = (struct exynos5420_tzasc *)samsung_get_base_dmc_tzasc();
	tzasc1 = (struct exynos5420_tzasc *)(samsung_get_base_dmc_tzasc()
							+ DMC_OFFSET);
	pmu = (struct exynos5_power *)EXYNOS5420_POWER_BASE;

	if (CONFIG_NR_DRAM_BANKS > 4) {
		/* Need both controllers. */
		mem->memcontrol |= DMC_MEMCONTROL_NUM_CHIP_2;
		mem->chips_per_channel = 2;
		mem->chips_to_configure = 2;
	} else {
		/* 2GB requires a single controller */
		mem->memcontrol |= DMC_MEMCONTROL_NUM_CHIP_1;
	}

	/* Enable PAUSE for DREX */
	setbits_le32(&clk->pause, ENABLE_BIT);

	/* Enable BYPASS mode */
	setbits_le32(&clk->bpll_con1, BYPASS_EN);

	writel(MUX_BPLL_SEL_FOUTBPLL, &clk->src_cdrex);
	do {
		val = readl(&clk->mux_stat_cdrex);
		val &= BPLL_SEL_MASK;
	} while (val != FOUTBPLL);

	clrbits_le32(&clk->bpll_con1, BYPASS_EN);

	/* Specify the DDR memory type as DDR3 */
	val = readl(&phy0_ctrl->phy_con0);
	val &= ~(PHY_CON0_CTRL_DDR_MODE_MASK << PHY_CON0_CTRL_DDR_MODE_SHIFT);
	val |= (DDR_MODE_DDR3 << PHY_CON0_CTRL_DDR_MODE_SHIFT);
	writel(val, &phy0_ctrl->phy_con0);

	val = readl(&phy1_ctrl->phy_con0);
	val &= ~(PHY_CON0_CTRL_DDR_MODE_MASK << PHY_CON0_CTRL_DDR_MODE_SHIFT);
	val |= (DDR_MODE_DDR3 << PHY_CON0_CTRL_DDR_MODE_SHIFT);
	writel(val, &phy1_ctrl->phy_con0);

	/* Set Read Latency and Burst Length for PHY0 and PHY1 */
	val = (mem->ctrl_bstlen << PHY_CON42_CTRL_BSTLEN_SHIFT) |
		(mem->ctrl_rdlat << PHY_CON42_CTRL_RDLAT_SHIFT);
	writel(val, &phy0_ctrl->phy_con42);
	writel(val, &phy1_ctrl->phy_con42);

	val = readl(&phy0_ctrl->phy_con26);
	val &= ~(T_WRDATA_EN_MASK << T_WRDATA_EN_OFFSET);
	val |= (T_WRDATA_EN_DDR3 << T_WRDATA_EN_OFFSET);
	writel(val, &phy0_ctrl->phy_con26);

	val = readl(&phy1_ctrl->phy_con26);
	val &= ~(T_WRDATA_EN_MASK << T_WRDATA_EN_OFFSET);
	val |= (T_WRDATA_EN_DDR3 << T_WRDATA_EN_OFFSET);
	writel(val, &phy1_ctrl->phy_con26);

	/*
	 * Set Driver strength for CK, CKE, CS & CA to 0x7
	 * Set Driver strength for Data Slice 0~3 to 0x7
	 */
	val = (0x7 << CA_CK_DRVR_DS_OFFSET) | (0x7 << CA_CKE_DRVR_DS_OFFSET) |
		(0x7 << CA_CS_DRVR_DS_OFFSET) | (0x7 << CA_ADR_DRVR_DS_OFFSET);
	val |= (0x7 << DA_3_DS_OFFSET) | (0x7 << DA_2_DS_OFFSET) |
		(0x7 << DA_1_DS_OFFSET) | (0x7 << DA_0_DS_OFFSET);
	writel(val, &phy0_ctrl->phy_con39);
	writel(val, &phy1_ctrl->phy_con39);

	/* ZQ Calibration */
	if (dmc_config_zq(mem, &phy0_ctrl->phy_con16, &phy1_ctrl->phy_con16,
			  &phy0_ctrl->phy_con17, &phy1_ctrl->phy_con17))
		return SETUP_ERR_ZQ_CALIBRATION_FAILURE;

	clrbits_le32(&phy0_ctrl->phy_con16, ZQ_CLK_DIV_EN);
	clrbits_le32(&phy1_ctrl->phy_con16, ZQ_CLK_DIV_EN);

	/* DQ Signal */
	val = readl(&phy0_ctrl->phy_con14);
	val |= mem->phy0_pulld_dqs;
	writel(val, &phy0_ctrl->phy_con14);
	val = readl(&phy1_ctrl->phy_con14);
	val |= mem->phy1_pulld_dqs;
	writel(val, &phy1_ctrl->phy_con14);

	val = MEM_TERM_EN | PHY_TERM_EN;
	writel(val, &drex0->phycontrol0);
	writel(val, &drex1->phycontrol0);

	writel(mem->concontrol |
		(mem->dfi_init_start << CONCONTROL_DFI_INIT_START_SHIFT) |
		(mem->rd_fetch << CONCONTROL_RD_FETCH_SHIFT),
		&drex0->concontrol);
	writel(mem->concontrol |
		(mem->dfi_init_start << CONCONTROL_DFI_INIT_START_SHIFT) |
		(mem->rd_fetch << CONCONTROL_RD_FETCH_SHIFT),
		&drex1->concontrol);

	do {
		val = readl(&drex0->phystatus);
	} while ((val & DFI_INIT_COMPLETE) != DFI_INIT_COMPLETE);
	do {
		val = readl(&drex1->phystatus);
	} while ((val & DFI_INIT_COMPLETE) != DFI_INIT_COMPLETE);

	clrbits_le32(&drex0->concontrol, DFI_INIT_START);
	clrbits_le32(&drex1->concontrol, DFI_INIT_START);

	update_reset_dll(&drex0->phycontrol0, DDR_MODE_DDR3);
	update_reset_dll(&drex1->phycontrol0, DDR_MODE_DDR3);

	/*
	 * Set Base Address:
	 * 0x2000_0000 ~ 0x5FFF_FFFF
	 * 0x6000_0000 ~ 0x9FFF_FFFF
	 */
	/* MEMBASECONFIG0 */
	val = DMC_MEMBASECONFIGX_CHIP_BASE(DMC_CHIP_BASE_0) |
		DMC_MEMBASECONFIGX_CHIP_MASK(DMC_CHIP_MASK);
	writel(val, &tzasc0->membaseconfig0);
	writel(val, &tzasc1->membaseconfig0);

	/* MEMBASECONFIG1 */
	val = DMC_MEMBASECONFIGX_CHIP_BASE(DMC_CHIP_BASE_1) |
		DMC_MEMBASECONFIGX_CHIP_MASK(DMC_CHIP_MASK);
	writel(val, &tzasc0->membaseconfig1);
	writel(val, &tzasc1->membaseconfig1);

	/*
	 * Memory Channel Inteleaving Size
	 * Ares Channel interleaving = 128 bytes
	 */
	/* MEMCONFIG0/1 */
	writel(mem->memconfig, &tzasc0->memconfig0);
	writel(mem->memconfig, &tzasc1->memconfig0);
	writel(mem->memconfig, &tzasc0->memconfig1);
	writel(mem->memconfig, &tzasc1->memconfig1);

	/* Precharge Configuration */
	writel(mem->prechconfig_tp_cnt << PRECHCONFIG_TP_CNT_SHIFT,
	       &drex0->prechconfig0);
	writel(mem->prechconfig_tp_cnt << PRECHCONFIG_TP_CNT_SHIFT,
	       &drex1->prechconfig0);

	/*
	 * TimingRow, TimingData, TimingPower and Timingaref
	 * values as per Memory AC parameters
	 */
	writel(mem->timing_ref, &drex0->timingref);
	writel(mem->timing_ref, &drex1->timingref);
	writel(mem->timing_row, &drex0->timingrow0);
	writel(mem->timing_row, &drex1->timingrow0);
	writel(mem->timing_data, &drex0->timingdata0);
	writel(mem->timing_data, &drex1->timingdata0);
	writel(mem->timing_power, &drex0->timingpower0);
	writel(mem->timing_power, &drex1->timingpower0);

	if (reset) {
		/*
		 * Send NOP, MRS and ZQINIT commands
		 * Sending MRS command will reset the DRAM. We should not be
		 * resetting the DRAM after resume, this will lead to memory
		 * corruption as DRAM content is lost after DRAM reset
		 */
		dmc_config_mrs(mem, &drex0->directcmd);
		dmc_config_mrs(mem, &drex1->directcmd);
	}

	/*
	 * Get PHY_CON13 from both phys.  Gate CLKM around reading since
	 * PHY_CON13 is glitchy when CLKM is running.  We're paranoid and
	 * wait until we get a "fine lock", though a coarse lock is probably
	 * OK (we only use the coarse numbers below).  We try to gate the
	 * clock for as short a time as possible in case SDRAM is somehow
	 * sensitive.  sdelay(10) in the loop is arbitrary to make sure
	 * there is some time for PHY_CON13 to get updated.  In practice
	 * no delay appears to be needed.
	 */
	val = readl(&clk->gate_bus_cdrex);
	while (true) {
		writel(val & ~0x1, &clk->gate_bus_cdrex);
		lock0_info = readl(&phy0_ctrl->phy_con13);
		writel(val, &clk->gate_bus_cdrex);

		if ((lock0_info & CTRL_FINE_LOCKED) == CTRL_FINE_LOCKED)
			break;

		sdelay(10);
	}
	while (true) {
		writel(val & ~0x2, &clk->gate_bus_cdrex);
		lock1_info = readl(&phy1_ctrl->phy_con13);
		writel(val, &clk->gate_bus_cdrex);

		if ((lock1_info & CTRL_FINE_LOCKED) == CTRL_FINE_LOCKED)
			break;

		sdelay(10);
	}

	if (!reset) {
		/*
		 * During Suspend-Resume & S/W-Reset, as soon as PMU releases
		 * pad retention, CKE goes high. This causes memory contents
		 * not to be retained during DRAM initialization. Therfore,
		 * there is a new control register(0x100431e8[28]) which lets us
		 * release pad retention and retain the memory content until the
		 * initialization is complete.
		 */
		writel(PAD_RETENTION_DRAM_COREBLK_VAL,
		       &power->pad_retention_dram_coreblk_option);
		do {
			val = readl(&power->pad_retention_dram_status);
		} while (val != 0x1);

		/*
		 * CKE PAD retention disables DRAM self-refresh mode.
		 * Send auto refresh command for DRAM refresh.
		 */
		for (i = 0; i < 128; i++) {
			for (chip = 0; chip < mem->chips_to_configure; chip++) {
				writel(DIRECT_CMD_REFA |
				       (chip << DIRECT_CMD_CHIP_SHIFT),
				       &drex0->directcmd);
				writel(DIRECT_CMD_REFA |
				       (chip << DIRECT_CMD_CHIP_SHIFT),
				       &drex1->directcmd);
			}
		}
	}

	if (mem->gate_leveling_enable) {
		writel(PHY_CON0_RESET_VAL, &phy0_ctrl->phy_con0);
		writel(PHY_CON0_RESET_VAL, &phy1_ctrl->phy_con0);

		setbits_le32(&phy0_ctrl->phy_con0, P0_CMD_EN);
		setbits_le32(&phy1_ctrl->phy_con0, P0_CMD_EN);

		val = PHY_CON2_RESET_VAL;
		val |= INIT_DESKEW_EN;
		writel(val, &phy0_ctrl->phy_con2);
		writel(val, &phy1_ctrl->phy_con2);

		val =  readl(&phy0_ctrl->phy_con1);
		val |= (RDLVL_PASS_ADJ_VAL << RDLVL_PASS_ADJ_OFFSET);
		writel(val, &phy0_ctrl->phy_con1);

		val =  readl(&phy1_ctrl->phy_con1);
		val |= (RDLVL_PASS_ADJ_VAL << RDLVL_PASS_ADJ_OFFSET);
		writel(val, &phy1_ctrl->phy_con1);

		n_lock_w_phy0 = (lock0_info & CTRL_LOCK_COARSE_MASK) >> 2;
		n_lock_r = readl(&phy0_ctrl->phy_con12);
		n_lock_r &= ~CTRL_DLL_ON;
		n_lock_r |= n_lock_w_phy0;
		writel(n_lock_r, &phy0_ctrl->phy_con12);

		n_lock_w_phy1 = (lock1_info & CTRL_LOCK_COARSE_MASK) >> 2;
		n_lock_r = readl(&phy1_ctrl->phy_con12);
		n_lock_r &= ~CTRL_DLL_ON;
		n_lock_r |= n_lock_w_phy1;
		writel(n_lock_r, &phy1_ctrl->phy_con12);

		val = (0x3 << DIRECT_CMD_BANK_SHIFT) | 0x4;
		for (chip = 0; chip < mem->chips_to_configure; chip++) {
			writel(val | (chip << DIRECT_CMD_CHIP_SHIFT),
			       &drex0->directcmd);
			writel(val | (chip << DIRECT_CMD_CHIP_SHIFT),
			       &drex1->directcmd);
		}

		setbits_le32(&phy0_ctrl->phy_con2, RDLVL_GATE_EN);
		setbits_le32(&phy1_ctrl->phy_con2, RDLVL_GATE_EN);

		setbits_le32(&phy0_ctrl->phy_con0, CTRL_SHGATE);
		setbits_le32(&phy1_ctrl->phy_con0, CTRL_SHGATE);

		val = readl(&phy0_ctrl->phy_con1);
		val &= ~(CTRL_GATEDURADJ_MASK);
		writel(val, &phy0_ctrl->phy_con1);

		val = readl(&phy1_ctrl->phy_con1);
		val &= ~(CTRL_GATEDURADJ_MASK);
		writel(val, &phy1_ctrl->phy_con1);

		writel(CTRL_RDLVL_GATE_ENABLE, &drex0->rdlvl_config);
		i = TIMEOUT_US;
		while (((readl(&drex0->phystatus) & RDLVL_COMPLETE_CHO) !=
			RDLVL_COMPLETE_CHO) && (i > 0)) {
			/*
			 * TODO(waihong): Comment on how long this take to
			 * timeout
			 */
			sdelay(100);
			i--;
		}
		if (!i)
			return SETUP_ERR_RDLV_COMPLETE_TIMEOUT;
		writel(CTRL_RDLVL_GATE_DISABLE, &drex0->rdlvl_config);

		writel(CTRL_RDLVL_GATE_ENABLE, &drex1->rdlvl_config);
		i = TIMEOUT_US;
		while (((readl(&drex1->phystatus) & RDLVL_COMPLETE_CHO) !=
			RDLVL_COMPLETE_CHO) && (i > 0)) {
			/*
			 * TODO(waihong): Comment on how long this take to
			 * timeout
			 */
			sdelay(100);
			i--;
		}
		if (!i)
			return SETUP_ERR_RDLV_COMPLETE_TIMEOUT;
		writel(CTRL_RDLVL_GATE_DISABLE, &drex1->rdlvl_config);

		writel(0, &phy0_ctrl->phy_con14);
		writel(0, &phy1_ctrl->phy_con14);

		val = (0x3 << DIRECT_CMD_BANK_SHIFT);
		for (chip = 0; chip < mem->chips_to_configure; chip++) {
			writel(val | (chip << DIRECT_CMD_CHIP_SHIFT),
			       &drex0->directcmd);
			writel(val | (chip << DIRECT_CMD_CHIP_SHIFT),
			       &drex1->directcmd);
		}

		/* Common Settings for Leveling */
		val = PHY_CON12_RESET_VAL;
		writel((val + n_lock_w_phy0), &phy0_ctrl->phy_con12);
		writel((val + n_lock_w_phy1), &phy1_ctrl->phy_con12);

		setbits_le32(&phy0_ctrl->phy_con2, DLL_DESKEW_EN);
		setbits_le32(&phy1_ctrl->phy_con2, DLL_DESKEW_EN);
	}

	/*
	 * Do software read leveling
	 *
	 * Do this before we turn on auto refresh since the auto refresh can
	 * be in conflict with the resync operation that's part of setting
	 * read leveling.
	 */
	if (!reset) {
		/* restore calibrated value after resume */
		dmc_set_read_offset_value(phy0_ctrl, readl(&pmu->pmu_spare1));
		dmc_set_read_offset_value(phy1_ctrl, readl(&pmu->pmu_spare2));
	} else {
		software_find_read_offset(phy0_ctrl, 0,
					  CTRL_LOCK_COARSE(lock0_info));
		software_find_read_offset(phy1_ctrl, 1,
					  CTRL_LOCK_COARSE(lock1_info));
		/* save calibrated value to restore after resume */
		writel(dmc_get_read_offset_value(phy0_ctrl), &pmu->pmu_spare1);
		writel(dmc_get_read_offset_value(phy1_ctrl), &pmu->pmu_spare2);
	}

	/* Send PALL command */
	dmc_config_prech(mem, &drex0->directcmd);
	dmc_config_prech(mem, &drex1->directcmd);

	writel(mem->memcontrol, &drex0->memcontrol);
	writel(mem->memcontrol, &drex1->memcontrol);

	/*
	 * Set DMC Concontrol: Enable auto-refresh counter, provide
	 * read data fetch cycles and enable DREX auto set powerdown
	 * for input buffer of I/O in none read memory state.
	 */
	writel(mem->concontrol | (mem->aref_en << CONCONTROL_AREF_EN_SHIFT) |
		(mem->rd_fetch << CONCONTROL_RD_FETCH_SHIFT)|
		DMC_CONCONTROL_IO_PD_CON(0x2),
		&drex0->concontrol);
	writel(mem->concontrol | (mem->aref_en << CONCONTROL_AREF_EN_SHIFT) |
		(mem->rd_fetch << CONCONTROL_RD_FETCH_SHIFT)|
		DMC_CONCONTROL_IO_PD_CON(0x2),
		&drex1->concontrol);

	/*
	 * Enable Clock Gating Control for DMC
	 * this saves around 25 mw dmc power as compared to the power
	 * consumption without these bits enabled
	 */
	setbits_le32(&drex0->cgcontrol, DMC_INTERNAL_CG);
	setbits_le32(&drex1->cgcontrol, DMC_INTERNAL_CG);

	/*
	 * As per Exynos5800 UM ver 0.00 section 17.13.2.1
	 * CONCONTROL register bit 3 [update_mode], Exynos5800 does not
	 * support the PHY initiated update. And it is recommended to set
	 * this field to 1'b1 during initialization
	 *
	 * When we apply PHY-initiated mode, DLL lock value is determined
	 * once at DMC init time and not updated later when we change the MIF
	 * voltage based on ASV group in kernel. Applying MC-initiated mode
	 * makes sure that DLL tracing is ON so that silicon is able to
	 * compensate the voltage variation.
	 */
	val = readl(&drex0->concontrol);
	val |= CONCONTROL_UPDATE_MODE;
	writel(val, &drex0->concontrol);
	val = readl(&drex1->concontrol);
	val |= CONCONTROL_UPDATE_MODE;
	writel(val, &drex1->concontrol);

	return 0;
}
#endif
