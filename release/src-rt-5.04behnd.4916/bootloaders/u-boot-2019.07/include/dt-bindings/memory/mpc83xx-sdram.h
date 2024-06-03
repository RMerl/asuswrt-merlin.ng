/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2018
 * Mario Six, Guntermann & Drunck GmbH, mario.six@gdsys.cc
 */

#ifndef DT_BINDINGS_MPC83XX_SDRAM_H
#define DT_BINDINGS_MPC83XX_SDRAM_H

/* DDR Control Driver register */

#define DSO_DISABLE	0
#define DSO_ENABLE	1

#define DSO_P_IMPEDANCE_HIGHEST_Z	0x0
#define DSO_P_IMPEDANCE_MUCH_HIGHER_Z	0x8
#define DSO_P_IMPEDANCE_HIGHER_Z	0xC
#define DSO_P_IMPEDANCE_NOMINAL		0xE
#define DSO_P_IMPEDANCE_LOWER_Z		0xF

#define DSO_N_IMPEDANCE_HIGHEST_Z	0x0
#define DSO_N_IMPEDANCE_MUCH_HIGHER_Z	0x8
#define DSO_N_IMPEDANCE_HIGHER_Z	0xC
#define DSO_N_IMPEDANCE_NOMINAL		0xE
#define DSO_N_IMPEDANCE_LOWER_Z		0xF

#define ODT_TERMINATION_75_OHM		0
#define ODT_TERMINATION_150_OHM		1

#define DDR_TYPE_DDR2_1_8_VOLT		0
#define DDR_TYPE_DDR1_2_5_VOLT		1

#define MVREF_SEL_EXTERNAL		0
#define MVREF_SEL_INTERNAL_GVDD		1

#define M_ODR_ENABLE			0
#define M_ODR_DISABLE			1

/* CS config register */

#define AUTO_PRECHARGE_ENABLE	0x00800000
#define AUTO_PRECHARGE_DISABLE	0x00000000

#define ODT_RD_NEVER		0x00000000
#define ODT_RD_ONLY_CURRENT	0x00100000
#define ODT_RD_ONLY_OTHER_CS	0x00200000
#define ODT_RD_ONLY_OTHER_DIMM	0x00300000
#define ODT_RD_ALL		0x00400000

#define ODT_WR_NEVER		0x00000000
#define ODT_WR_ONLY_CURRENT	0x00010000
#define ODT_WR_ONLY_OTHER_CS	0x00020000
#define ODT_WR_ONLY_OTHER_DIMM	0x00030000
#define ODT_WR_ALL		0x00040000

/* DDR SDRAM Clock Control register */

#define CLOCK_ADJUST_025	0x01000000
#define CLOCK_ADJUST_05		0x02000000
#define CLOCK_ADJUST_075	0x03000000
#define CLOCK_ADJUST_1		0x04000000

#define CASLAT_20		0x3	/* CAS latency = 2.0 */
#define CASLAT_25		0x4	/* CAS latency = 2.5 */
#define CASLAT_30		0x5	/* CAS latency = 3.0 */
#define CASLAT_35		0x6	/* CAS latency = 3.5 */
#define CASLAT_40		0x7	/* CAS latency = 4.0 */
#define CASLAT_45		0x8	/* CAS latency = 4.5 */
#define CASLAT_50		0x9	/* CAS latency = 5.0 */
#define CASLAT_55		0xa	/* CAS latency = 5.5 */
#define CASLAT_60		0xb	/* CAS latency = 6.0 */
#define CASLAT_65		0xc	/* CAS latency = 6.5 */
#define CASLAT_70		0xd	/* CAS latency = 7.0 */
#define CASLAT_75		0xe	/* CAS latency = 7.5 */
#define CASLAT_80		0xf	/* CAS latency = 8.0 */

/* DDR SDRAM Timing Configuration 2 register */

#define READ_LAT_PLUS_1		0x0
#define READ_LAT		0x2
#define READ_LAT_PLUS_1_4	0x3
#define READ_LAT_PLUS_1_2	0x4
#define READ_LAT_PLUS_3_4	0x5
#define READ_LAT_PLUS_5_4	0x7
#define READ_LAT_PLUS_3_2	0x8
#define READ_LAT_PLUS_7_4	0x9
#define READ_LAT_PLUS_2		0xA
#define READ_LAT_PLUS_9_4	0xB
#define READ_LAT_PLUS_5_2	0xC
#define READ_LAT_PLUS_11_4	0xD
#define READ_LAT_PLUS_3		0xE
#define READ_LAT_PLUS_13_4	0xF
#define READ_LAT_PLUS_7_2	0x10
#define READ_LAT_PLUS_15_4	0x11
#define READ_LAT_PLUS_4		0x12
#define READ_LAT_PLUS_17_4	0x13
#define READ_LAT_PLUS_9_2	0x14
#define READ_LAT_PLUS_19_4	0x15

#define CLOCK_DELAY_0		0x0
#define CLOCK_DELAY_1_4		0x1
#define CLOCK_DELAY_1_2		0x2
#define CLOCK_DELAY_3_4		0x3
#define CLOCK_DELAY_1		0x4
#define CLOCK_DELAY_5_4		0x5
#define CLOCK_DELAY_3_2		0x6

/* DDR SDRAM Control Configuration */

#define SREN_DISABLE	0x0
#define SREN_ENABLE	0x1

#define ECC_DISABLE	0x0
#define ECC_ENABLE	0x1

#define RD_DISABLE	0x0
#define RD_ENABLE	0x1

#define TYPE_DDR1	0x2
#define TYPE_DDR2	0x3

#define DYN_PWR_DISABLE		0x0
#define DYN_PWR_ENABLE		0x1

#define DATA_BUS_WIDTH_16	0x1
#define DATA_BUS_WIDTH_32	0x2

#define NCAP_DISABLE	0x0
#define NCAP_ENABLE	0x1

#define TIMING_1T	0x0
#define TIMING_2T	0x1

#define INTERLEAVE_NONE		0x0
#define INTERLEAVE_1_AND_2	0x1

#define PRECHARGE_MA_10		0x0
#define PRECHARGE_MA_8		0x1

#define STRENGTH_FULL		0x0
#define STRENGTH_HALF		0x1

#define INITIALIZATION_DONT_BYPASS	0x0
#define INITIALIZATION_BYPASS		0x1

/* DDR SDRAM Control Configuration 2 register */

#define MODE_NORMAL	0x0
#define MODE_REFRESH	0x1

#define DLL_RESET_ENABLE	0x0
#define DLL_RESET_DISABLE	0x1

#define DQS_TRUE	0x0

#define ODT_ASSERT_NEVER	0x0
#define ODT_ASSERT_WRITES	0x1
#define ODT_ASSERT_READS	0x2
#define ODT_ASSERT_ALWAYS	0x3

#endif
