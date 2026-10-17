/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 MediaTek Inc.
 *
 * Author:  Weijie Gao <weijie.gao@mediatek.com>
 */

#ifndef _MTMIPS_MC_H_
#define _MTMIPS_MC_H_

#define MEMCTL_SDRAM_CFG0_REG		0x00
#define DIS_CLK_GT			0x80000000
#define CLK_SLEW_S			29
#define CLK_SLEW_M			0x60000000
#define TWR				0x10000000
#define TMRD_S				24
#define TMRD_M				0xf000000
#define TRFC_S				20
#define TRFC_M				0xf00000
#define TCAS_S				16
#define TCAS_M				0x30000
#define TRAS_S				12
#define TRAS_M				0xf000
#define TRCD_S				8
#define TRCD_M				0x300
#define TRC_S				4
#define TRC_M				0xf0
#define TRP_S				0
#define TRP_M				0x03

#define MEMCTL_SDRAM_CFG1_REG		0x04
#define SDRAM_INIT_START		0x80000000
#define SDRAM_INIT_DONE			0x40000000
#define RBC_MAPPING			0x20000000
#define PWR_DOWN_EN			0x10000000
#define PWR_DOWN_MODE			0x8000000
#define SDRAM_WIDTH			0x1000000
#define NUMCOLS_S			20
#define NUMCOLS_M			0x300000
#define NUMROWS_S			16
#define NUMROWS_M			0x30000
#define TREFR_S				0
#define TREFR_M				0xffff

#define MEMCTL_DDR_SELF_REFRESH_REG	0x10
#define ODT_SRC_SEL_S			24
#define ODT_SRC_SEL_M			0xf000000
#define ODT_OFF_DLY_S			20
#define ODT_OFF_DLY_M			0xf00000
#define ODT_ON_DLY_S			16
#define ODT_ON_DLY_M			0xf0000
#define SR_AUTO_EN			0x10
#define SRACK_B				0x02
#define SRREQ_B				0x01

#define MEMCTL_PWR_SAVE_CNT_REG		0x14
#define PD_CNT_S			24
#define PD_CNT_M			0xff000000
#define SR_TAR_CNT_S			0
#define SR_TAR_CNT_M			0xffffff

#define MEMCTL_DLL_DBG_REG		0x20
#define TDC_STABLE_S			12
#define TDC_STABLE_M			0x3f000
#define MST_DLY_SEL_S			4
#define MST_DLY_SEL_M			0xff0
#define CURR_STATE_S			1
#define CURR_STATE_M			0x06
#define ADLL_LOCK_DONE			0x01

#define MEMCTL_DDR_CFG0_REG		0x40
#define T_RRD_S				28
#define T_RRD_M				0xf0000000
#define T_RAS_S				23
#define T_RAS_M				0xf800000
#define T_RP_S				19
#define T_RP_M				0x780000
#define T_RFC_S				13
#define T_RFC_M				0x7e000
#define T_REFI_S			0
#define T_REFI_M			0x1fff

#define MEMCTL_DDR_CFG1_REG		0x44
#define T_WTR_S				28
#define T_WTR_M				0xf0000000
#define T_RTP_S				24
#define T_RTP_M				0xf000000
#define USER_DATA_WIDTH			0x200000
#define IND_SDRAM_SIZE_S		18
#define IND_SDRAM_SIZE_M		0x1c0000
#define IND_SDRAM_SIZE_8MB		1
#define IND_SDRAM_SIZE_16MB		2
#define IND_SDRAM_SIZE_32MB		3
#define IND_SDRAM_SIZE_64MB		4
#define IND_SDRAM_SIZE_128MB		5
#define IND_SDRAM_SIZE_256MB		6
#define IND_SDRAM_WIDTH_S		16
#define IND_SDRAM_WIDTH_M		0x30000
#define IND_SDRAM_WIDTH_8BIT		1
#define IND_SDRAM_WIDTH_16BIT		2
#define EXT_BANK_S			14
#define EXT_BANK_M			0xc000
#define TOTAL_SDRAM_WIDTH_S		12
#define TOTAL_SDRAM_WIDTH_M		0x3000
#define T_WR_S				8
#define T_WR_M				0xf00
#define T_MRD_S				4
#define T_MRD_M				0xf0
#define T_RCD_S				0
#define T_RCD_M				0x0f

#define MEMCTL_DDR_CFG2_REG		0x48
#define REGE				0x80000000
#define DDR2_MODE			0x40000000
#define DQS0_GATING_WINDOW_S		28
#define DQS0_GATING_WINDOW_M		0x30000000
#define DQS1_GATING_WINDOW_S		26
#define DQS1_GATING_WINDOW_M		0xc000000
#define PD				0x1000
#define WR_S				9
#define WR_M				0xe00
#define DLLRESET			0x100
#define TESTMODE			0x80
#define CAS_LATENCY_S			4
#define CAS_LATENCY_M			0x70
#define BURST_TYPE			0x08
#define BURST_LENGTH_S			0
#define BURST_LENGTH_M			0x07

#define MEMCTL_DDR_CFG3_REG		0x4c
#define Q_OFF				0x1000
#define RDOS				0x800
#define DIS_DIFF_DQS			0x400
#define OCD_S				7
#define OCD_M				0x380
#define RTT1				0x40
#define ADDITIVE_LATENCY_S		3
#define ADDITIVE_LATENCY_M		0x38
#define RTT0				0x04
#define DS				0x02
#define DLL				0x01

#define MEMCTL_DDR_CFG4_REG		0x50
#define FAW_S				0
#define FAW_M				0x0f

#define MEMCTL_DDR_DQ_DLY_REG		0x60
#define DQ1_DELAY_SEL_S			24
#define DQ1_DELAY_SEL_M			0xff000000
#define DQ0_DELAY_SEL_S			16
#define DQ0_DELAY_SEL_M			0xff0000
#define DQ1_DELAY_COARSE_TUNING_S	12
#define DQ1_DELAY_COARSE_TUNING_M	0xf000
#define DQ1_DELAY_FINE_TUNING_S		8
#define DQ1_DELAY_FINE_TUNING_M		0xf00
#define DQ0_DELAY_COARSE_TUNING_S	4
#define DQ0_DELAY_COARSE_TUNING_M	0xf0
#define DQ0_DELAY_FINE_TUNING_S		0
#define DQ0_DELAY_FINE_TUNING_M		0x0f

#define MEMCTL_DDR_DQS_DLY_REG		0x64
#define DQS1_DELAY_SEL_S		24
#define DQS1_DELAY_SEL_M		0xff000000
#define DQS0_DELAY_SEL_S		16
#define DQS0_DELAY_SEL_M		0xff0000
#define DQS1_DELAY_COARSE_TUNING_S	12
#define DQS1_DELAY_COARSE_TUNING_M	0xf000
#define DQS1_DELAY_FINE_TUNING_S	8
#define DQS1_DELAY_FINE_TUNING_M	0xf00
#define DQS0_DELAY_COARSE_TUNING_S	4
#define DQS0_DELAY_COARSE_TUNING_M	0xf0
#define DQS0_DELAY_FINE_TUNING_S	0
#define DQS0_DELAY_FINE_TUNING_M	0x0f

#define MEMCTL_DDR_DLL_SLV_REG		0x68
#define DLL_SLV_UPDATE_MODE		0x100
#define DQS_DLY_SEL_EN			0x80
#define DQ_DLY_SEL_EN			0x01

#endif /* _MTMIPS_MC_H_ */
