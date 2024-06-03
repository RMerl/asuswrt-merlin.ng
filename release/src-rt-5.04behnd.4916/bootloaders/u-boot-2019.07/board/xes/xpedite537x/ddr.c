// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2008 Extreme Engineering Solutions, Inc.
 * Copyright 2008 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <i2c.h>

#include <fsl_ddr_sdram.h>
#include <fsl_ddr_dimm_params.h>

void get_spd(ddr2_spd_eeprom_t *spd, u8 i2c_address)
{
	i2c_read(i2c_address, SPD_EEPROM_OFFSET, 2, (uchar *)spd,
		 sizeof(ddr2_spd_eeprom_t));
}

/*
 * There are four board-specific SDRAM timing parameters which must be
 * calculated based on the particular PCB artwork.  These are:
 *   1.) CPO (Read Capture Delay)
 *           - TIMING_CFG_2 register
 *           Source: Calculation based on board trace lengths and
 *                   chip-specific internal delays.
 *   2.) WR_DATA_DELAY (Write Command to Data Strobe Delay)
 *           - TIMING_CFG_2 register
 *           Source: Calculation based on board trace lengths.
 *                   Unless clock and DQ lanes are very different
 *                   lengths (>2"), this should be set to the nominal value
 *                   of 1/2 clock delay.
 *   3.) CLK_ADJUST (Clock and Addr/Cmd alignment control)
 *           - DDR_SDRAM_CLK_CNTL register
 *           Source: Signal Integrity Simulations
 *   4.) 2T Timing on Addr/Ctl
 *           - TIMING_CFG_2 register
 *           Source: Signal Integrity Simulations
 *           Usually only needed with heavy load/very high speed (>DDR2-800)
 *
 *     ====== XPedite5370 DDR2-600 read delay calculations ======
 *
 *     See Freescale's App Note AN2583 as refrence.  This document also
 *     contains the chip-specific delays for 8548E, 8572, etc.
 *
 *     For MPC8572E
 *     Minimum chip delay (Ch 0): 1.372ns
 *     Maximum chip delay (Ch 0): 2.914ns
 *     Minimum chip delay (Ch 1): 1.220ns
 *     Maximum chip delay (Ch 1): 2.595ns
 *
 *     CLK adjust = 5 (from simulations) = 5/8* 3.33ns = 2080ps
 *
 *     Minimum delay calc (Ch 0):
 *     clock prop - dram skew + min dqs prop delay + clk_adjust + min chip dly
 *     2.3" * 180 - 400ps     + 1.9" * 180         + 2080ps     + 1372ps
 *                                                 = 3808ps
 *                                                 = 3.808ns
 *
 *     Maximum delay calc (Ch 0):
 *     clock prop + dram skew + max dqs prop delay + clk_adjust + max chip dly
 *     2.3" * 180 + 400ps     + 2.4" * 180         + 2080ps     + 2914ps
 *                                                 = 6240ps
 *                                                 = 6.240ns
 *
 *     Minimum delay calc (Ch 1):
 *     clock prop - dram skew + min dqs prop delay + clk_adjust + min chip dly
 *     1.46" * 180- 400ps     + 0.7" * 180         + 2080ps     + 1220ps
 *                                                 = 3288ps
 *                                                 = 3.288ns
 *
 *     Maximum delay calc (Ch 1):
 *     clock prop + dram skew + max dqs prop delay + clk_adjust + min chip dly
 *     1.46" * 180+ 400ps     + 1.1" * 180         + 2080ps     + 2595ps
 *                                                 = 5536ps
 *                                                 = 5.536ns
 *
 *     Ch.0: 3.808ns to 6.240ns additional delay needed  (pick 5ns as target)
 *              This is 1.5 clock cycles, pick CPO = READ_LAT + 3/2 (0x8)
 *     Ch.1: 3.288ns to 5.536ns additional delay needed  (pick 4.4ns as target)
 *              This is 1.32 clock cycles, pick CPO = READ_LAT + 5/4 (0x7)
 *
 *
 *     ====== XPedite5370 DDR2-800 read delay calculations ======
 *
 *     See Freescale's App Note AN2583 as refrence.  This document also
 *     contains the chip-specific delays for 8548E, 8572, etc.
 *
 *     For MPC8572E
 *     Minimum chip delay (Ch 0): 1.372ns
 *     Maximum chip delay (Ch 0): 2.914ns
 *     Minimum chip delay (Ch 1): 1.220ns
 *     Maximum chip delay (Ch 1): 2.595ns
 *
 *     CLK adjust = 5 (from simulations) = 5/8* 2.5ns = 1563ps
 *
 *     Minimum delay calc (Ch 0):
 *     clock prop - dram skew + min dqs prop delay + clk_adjust + min chip dly
 *     2.3" * 180 - 350ps     + 1.9" * 180         + 1563ps     + 1372ps
 *                                                 = 3341ps
 *                                                 = 3.341ns
 *
 *     Maximum delay calc (Ch 0):
 *     clock prop + dram skew + max dqs prop delay + clk_adjust + max chip dly
 *     2.3" * 180 + 350ps     + 2.4" * 180         + 1563ps     + 2914ps
 *                                                 = 5673ps
 *                                                 = 5.673ns
 *
 *     Minimum delay calc (Ch 1):
 *     clock prop - dram skew + min dqs prop delay + clk_adjust + min chip dly
 *     1.46" * 180- 350ps     + 0.7" * 180         + 1563ps     + 1220ps
 *                                                 = 2822ps
 *                                                 = 2.822ns
 *
 *     Maximum delay calc (Ch 1):
 *     clock prop + dram skew + max dqs prop delay + clk_adjust + min chip dly
 *     1.46" * 180+ 350ps     + 1.1" * 180         + 1563ps     + 2595ps
 *                                                 = 4968ps
 *                                                 = 4.968ns
 *
 *     Ch.0: 3.341ns to 5.673ns additional delay needed  (pick 4.5ns as target)
 *              This is 1.8 clock cycles, pick CPO = READ_LAT + 7/4 (0x9)
 *     Ch.1: 2.822ns to 4.968ns additional delay needed  (pick 3.9ns as target)
 *              This is 1.56 clock cycles, pick CPO = READ_LAT + 3/2 (0x8)
 *
 * Write latency (WR_DATA_DELAY) is calculated by doing the following:
 *
 *      The DDR SDRAM specification requires DQS be received no sooner than
 *      75% of an SDRAM clock periodÂ—and no later than 125% of a clock
 *      periodÂ—from the capturing clock edge of the command/address at the
 *      SDRAM.
 *
 * Based on the above tracelengths, the following are calculated:
 *      Ch. 0 8572 to DRAM propagation (DQ lanes) : 1.9" * 180 =  0.342ns
 *      Ch. 0 8572 to DRAM propagation (CLKs) :     2.3" * 180 =  0.414ns
 *      Ch. 1 8572 to DRAM propagation (DQ lanes) : 0.7" * 180 =  0.126ns
 *      Ch. 1 8572 to DRAM propagation (CLKs   ) : 1.47" * 180 =  0.264ns
 *
 * Difference in arrival time CLK vs. DQS:
 *      Ch. 0 0.072ns
 *      Ch. 1 0.138ns
 *
 *      Both of these values are much less than 25% of the clock
 *      period at DDR2-600 or DDR2-800, so no additional delay is needed over
 *      the 1/2 cycle which normally aligns the first DQS transition
 *      exactly WL (CAS latency minus one cycle) after the CAS strobe.
 *      See Figure 9-53 in MPC8572E manual: "1/2 delay" in Freescale's
 *      terminology corresponds to exactly one clock period delay after
 *      the CAS strobe. (due to the fact that the "delay" is referenced
 *      from the *falling* edge of the CLK, just after the rising edge
 *      which the CAS strobe is latched on.
 */

typedef struct board_memctl_options {
	uint16_t datarate_mhz_low;
	uint16_t datarate_mhz_high;
	uint8_t clk_adjust;
	uint8_t cpo_override;
	uint8_t write_data_delay;
} board_memctl_options_t;

static struct board_memctl_options bopts_ctrl[][2] = {
	{
		/* Controller 0 */
		{
			/* DDR2 600/667 */
			.datarate_mhz_low	= 500,
			.datarate_mhz_high	= 750,
			.clk_adjust		= 5,
			.cpo_override		= 8,
			.write_data_delay	= 2,
		},
		{
			/* DDR2 800 */
			.datarate_mhz_low	= 750,
			.datarate_mhz_high	= 850,
			.clk_adjust		= 5,
			.cpo_override		= 9,
			.write_data_delay	= 2,
		},
	},
	{
		/* Controller 1 */
		{
			/* DDR2 600/667 */
			.datarate_mhz_low	= 500,
			.datarate_mhz_high	= 750,
			.clk_adjust		= 5,
			.cpo_override		= 7,
			.write_data_delay	= 2,
		},
		{
			/* DDR2 800 */
			.datarate_mhz_low	= 750,
			.datarate_mhz_high	= 850,
			.clk_adjust		= 5,
			.cpo_override		= 8,
			.write_data_delay	= 2,
		},
	},
};

void fsl_ddr_board_options(memctl_options_t *popts,
			   dimm_params_t *pdimm,
			   unsigned int ctrl_num)
{
	struct board_memctl_options *bopts = bopts_ctrl[ctrl_num];
	sys_info_t sysinfo;
	int i;
	unsigned int datarate;

	get_sys_info(&sysinfo);
	datarate = sysinfo.freq_ddrbus / 1000 / 1000;

	for (i = 0; i < ARRAY_SIZE(bopts_ctrl[ctrl_num]); i++) {
		if ((bopts[i].datarate_mhz_low <= datarate) &&
		    (bopts[i].datarate_mhz_high >= datarate)) {
			debug("controller %d:\n", ctrl_num);
			debug(" clk_adjust = %d\n", bopts[i].clk_adjust);
			debug(" cpo = %d\n", bopts[i].cpo_override);
			debug(" write_data_delay = %d\n",
			      bopts[i].write_data_delay);
			popts->clk_adjust = bopts[i].clk_adjust;
			popts->cpo_override = bopts[i].cpo_override;
			popts->write_data_delay = bopts[i].write_data_delay;
		}
	}

	/*
	 * Factors to consider for half-strength driver enable:
	 *	- number of DIMMs installed
	 */
	popts->half_strength_driver_enable = 0;
}
