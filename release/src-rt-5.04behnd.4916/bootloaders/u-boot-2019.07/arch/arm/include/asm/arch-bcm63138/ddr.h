/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2019 Broadcom Ltd.
 */

#ifndef _63138_DDR_H
#define _63138_DDR_H

#define DDRPHY_BASE    0x80003000
#define MEMC_BASE      0x80002000

typedef struct UBUSInterface {
	uint32_t CFG;		/* 0x00 */
#define UBUSIF_CFG_WRITE_REPLY_MODE_SHIFT   0x0
#define UBUSIF_CFG_WRITE_REPLY_MODE_MASK    (0x1<< UBUSIF_CFG_WRITE_REPLY_MODE_SHIFT)
#define UBUSIF_CFG_WRITE_BURST_MODE_SHIFT   0x1
#define UBUSIF_CFG_WRITE_BURST_MODE_MASK    (0x1<< UBUSIF_CFG_WRITE_BURST_MODE_SHIFT)
#define UBUSIF_CFG_INBAND_ERR_MASK_SHIFT    0x2
#define UBUSIF_CFG_INBAND_ERR_MASK_MASK     (0x1<< UBUSIF_CFG_INBAND_ERR_MASK_SHIFT)
#define UBUSIF_CFG_OOB_ERR_MASK_SHIFT       0x3
#define UBUSIF_CFG_OOB_ERR_MASK_MASK        (0x1<< UBUSIF_CFG_OOB_ERR_MASK_SHIFT)
	uint32_t SRC_QUEUE_CTRL_0;	/* 0x04 */
	uint32_t SRC_QUEUE_CTRL_1;	/* 0x08 */
	uint32_t SRC_QUEUE_CTRL_2;	/* 0x0c */
	uint32_t SRC_QUEUE_CTRL_3;	/* 0x10 */
	uint32_t REP_ARB_MODE;	/* 0x14 */
#define UBUSIF_REP_ARB_MODE_FIFO_MODE_SHIFT 0x0
#define UBUSIF_REP_ARB_MODE_FIFO_MODE_MASK  (0x1<<UBUSIF_REP_ARB_MODE_FIFO_MODE_SHIFT)
	uint32_t SCRATCH;	/* 0x18 */
	uint32_t DEBUG_R0;	/* 0x1c */
	uint32_t unused[8];	/* 0x20-0x3f */
} UBUSInterface;

typedef struct EDISEngine {
	uint32_t REV_ID;	/* 0x00 */
	uint32_t CTRL_TRIG;	/* 0x04 */
	uint32_t CTRL_MODE;	/* 0x08 */
	uint32_t CTRL_SIZE;	/* 0x0c */
	uint32_t CTRL_ADDR_START;	/* 0x10 */
	uint32_t CTRL_ADDR_START_EXT;	/* 0x14 */
	uint32_t CTRL_ADDR_END;	/* 0x18 */
	uint32_t CTRL_ADDR_END_EXT;	/* 0x1c */
	uint32_t CTRL_WRITE_MASKS;	/* 0x20 */
	uint32_t unused0[7];	/* 0x24-0x3f */
	uint32_t STAT_MAIN;	/* 0x40 */
	uint32_t STAT_WORDS_WRITTEN;	/* 0x44 */
	uint32_t STAT_WORDS_READ;	/* 0x48 */
	uint32_t STAT_ERROR_COUNT;	/* 0x4c */
	uint32_t STAT_ERROR_BITS;	/* 0x50 */
	uint32_t STAT_ADDR_LAST;	/* 0x54 */
	uint32_t STAT_ADDR_LAST_EXT;	/* 0x58 */
	uint32_t unused1[8];	/* 0x5c-0x7b */
	uint32_t STAT_DEBUG;	/* 0x7c */
	uint32_t STAT_DATA_PORT[8];	/* 0x80-0x9c */
	uint32_t GEN_LFSR_STATE[4];	/* 0xa0-0xac */
	uint32_t GEN_CLOCK;	/* 0xb0 */
	uint32_t GEN_PATTERN;	/* 0xb4 */
	uint32_t unused2[2];	/* 0xb8-0xbf */
	uint32_t BYTELANE_0_CTRL_LO;	/* 0xc0 */
	uint32_t BYTELANE_0_CTRL_HI;	/* 0xc4 */
	uint32_t BYTELANE_1_CTRL_LO;	/* 0xc8 */
	uint32_t BYTELANE_1_CTRL_HI;	/* 0xcc */
	uint32_t BYTELANE_2_CTRL_LO;	/* 0xd0 */
	uint32_t BYTELANE_2_CTRL_HI;	/* 0xd4 */
	uint32_t BYTELANE_3_CTRL_LO;	/* 0xd8 */
	uint32_t BYTELANE_3_CTRL_HI;	/* 0xdc */
	uint32_t BYTELANE_0_STAT_LO;	/* 0xe0 */
	uint32_t BYTELANE_0_STAT_HI;	/* 0xe4 */
	uint32_t BYTELANE_1_STAT_LO;	/* 0xe8 */
	uint32_t BYTELANE_1_STAT_HI;	/* 0xec */
	uint32_t BYTELANE_2_STAT_LO;	/* 0xf0 */
	uint32_t BYTELANE_2_STAT_HI;	/* 0xf4 */
	uint32_t BYTELANE_3_STAT_LO;	/* 0xf8 */
	uint32_t BYTELANE_3_STAT_HI;	/* 0xfc */
} EDISEngine;

typedef struct SecureRangeCheckers {
	uint32_t LOCK;		/* 0x00 */
	uint32_t LOG_INFO_0;	/* 0x04 */
	uint32_t LOG_INFO_1;	/* 0x08 */
	uint32_t CTRL_0;	/* 0x0c */
	uint32_t UBUS0_PORT_0;	/* 0x10 */
	uint32_t BASE_0;	/* 0x14 */
	uint32_t CTRL_1;	/* 0x18 */
	uint32_t UBUS0_PORT_1;	/* 0x1c */
	uint32_t BASE_1;	/* 0x20 */
	uint32_t CTRL_2;	/* 0x24 */
	uint32_t UBUS0_PORT_2;	/* 0x28 */
	uint32_t BASE_2;	/* 0x2c */
	uint32_t CTRL_3;	/* 0x30 */
	uint32_t UBUS0_PORT_3;	/* 0x34 */
	uint32_t BASE_3;	/* 0x38 */
	uint32_t CTRL_4;	/* 0x3c */
	uint32_t UBUS0_PORT_4;	/* 0x40 */
	uint32_t BASE_4;	/* 0x44 */
	uint32_t CTRL_5;	/* 0x48 */
	uint32_t UBUS0_PORT_5;	/* 0x4c */
	uint32_t BASE_5;	/* 0x50 */
	uint32_t CTRL_6;	/* 0x54 */
	uint32_t UBUS0_PORT_6;	/* 0x58 */
	uint32_t BASE_6;	/* 0x5c */
	uint32_t CTRL_7;	/* 0x60 */
	uint32_t UBUS0_PORT_7;	/* 0x64 */
	uint32_t BASE_7;	/* 0x68 */
	uint32_t unused[37];	/* 0x6c-0xff */
} SecureRangeCheckers;

typedef struct DDRPhyControl {
	uint32_t REVISION;	/* 0x00 */
	uint32_t PLL_STATUS;	/* 0x04 */
	uint32_t PLL_CONFIG;	/* 0x08 */
	uint32_t PLL_CONTROL1;	/* 0x0c */
	uint32_t PLL_CONTROL2;	/* 0x10 */
	uint32_t PLL_CONTROL3;	/* 0x14 */
	uint32_t PLL_DIVIDER;	/* 0x18 */
	uint32_t PLL_PRE_DIVIDER;	/* 0x1c */
	uint32_t PLL_SS_EN;	/* 0x20 */
	uint32_t PLL_SS_CFG;	/* 0x24 */
	uint32_t AUX_CONTROL;	/* 0x28 */
	uint32_t IDLE_PAD_CONTROL;	/* 0x2c */
	uint32_t IDLE_PAD_EN0;	/* 0x30 */
	uint32_t IDLE_PAD_EN1;	/* 0x34 */
	uint32_t DRIVE_PAD_CTL;	/* 0x38 */
	uint32_t STATIC_PAD_CTL;	/* 0x3c */
	uint32_t DRAM_CFG;	/* 0x40 */
	uint32_t DRAM_TIMING1;	/* 0x44 */
	uint32_t DRAM_TIMING2;	/* 0x48 */
	uint32_t DRAM_TIMING3;	/* 0x4c */
	uint32_t DRAM_TIMING4;	/* 0x50 */
	uint32_t unused0[3];	/* 0x54-0x5f */

	uint32_t VDL_REGS[47];	/* 0x60-0x118 */
	uint32_t AC_SPARE;	/* 0x11c */
	uint32_t unused1[4];	/* 0x120-0x12f */

	uint32_t REFRESH;	/* 0x130 */
	uint32_t UPDATE_VDL;	/* 0x134 */
	uint32_t UPDATE_VDL_SNOOP1;	/* 0x138 */
	uint32_t UPDATE_VDL_SNOOP2;	/* 0x13c */
	uint32_t CMND_REG1;	/* 0x140 */
	uint32_t CMND_AUX_REG1;	/* 0x144 */
	uint32_t CMND_REG2;	/* 0x148 */
	uint32_t CMND_AUX_REG2;	/* 0x14c */
	uint32_t CMND_REG3;	/* 0x150 */
	uint32_t CMND_AUX_REG3;	/* 0x154 */
	uint32_t CMND_REG4;	/* 0x158 */
	uint32_t CMND_AUX_REG4;	/* 0x15c */
	uint32_t CMND_REG_TIMER;	/* 0x160 */
	uint32_t MODE_REG[9];	/* 0x164-184 */
#define PHY_CONTROL_MODE_REG_VALID_SHIFT       16
#define PHY_CONTROL_MODE_REG_VALID_MASK        (0x1<<PHY_CONTROL_MODE_REG_VALID_SHIFT)
	uint32_t MODE_REG15;	/* 0x188 */
	uint32_t MODE_REG63;	/* 0x18c */
	uint32_t ALERT_CLEAR;	/* 0x190 */
	uint32_t ALERT_STATUS;	/* 0x194 */
	uint32_t CA_PARITY;	/* 0x198 */
	uint32_t CA_PLAYBACK_CTRL;	/* 0x19c */
	uint32_t CA_PLAYBACK_STATUS0;	/* 0x1a0 */
	uint32_t CA_PLAYBACK_STATUS1;	/* 0x1a4 */
	uint32_t unused2;	/* 0x1a8 */
	uint32_t WRITE_LEVEL_CTRL;	/* 0x1ac */
	uint32_t WRITE_LEVEL_STATUS;	/* 0x1b0 */
	uint32_t READ_EN_CTRL;	/* 0x1b4 */
	uint32_t READ_EN_STATUS;	/* 0x1b8 */
	uint32_t unused3;	/* 0x1bc */
	uint32_t TRAFFIC_GEN[12];	/* 0x1c0-0x1ec */
	uint32_t VIRT_VTT_CTRL;	/* 0x1f0 */
	uint32_t VIRT_VTT_STATUS;	/* 0x1f4 */
	uint32_t VIRT_VTT_CONNECTION;	/* 0x1f8 */
	uint32_t VIRT_VTT_OVERRIDE;	/* 0x1fc */
	uint32_t VREF_DAC_CTRL;	/* 0x200 */
	uint32_t PHYBIST[9];	/* 0x204-0x224 */
	uint32_t unused4[2];	/* 0x228-0x22f */
	uint32_t STANDBY_CTRL;	/* 0x230 */
	uint32_t DEBUG_FREEZE_EN;	/* 0x234 */
	uint32_t DEBUG_MUX_CTRL;	/* 0x238 */
	uint32_t DFI_CTRL;	/* 0x23c */
#define PHY_CONTROL_DFI_CNTRL_DFI_CS1_SHIFT    7
#define PHY_CONTROL_DFI_CNTRL_DFI_CS1_MASK     (0x1<<PHY_CONTROL_DFI_CNTRL_DFI_CS1_SHIFT)
#define PHY_CONTROL_DFI_CNTRL_DFI_CS0_SHIFT    6
#define PHY_CONTROL_DFI_CNTRL_DFI_CS0_MASK     (0x1<<PHY_CONTROL_DFI_CNTRL_DFI_CS0_SHIFT)
#define PHY_CONTROL_DFI_CNTRL_DFI_RSTN_SHIFT   5
#define PHY_CONTROL_DFI_CNTRL_DFI_RSTN_MASK    (0x1<<PHY_CONTROL_DFI_CNTRL_DFI_RSTN_SHIFT)
#define PHY_CONTROL_DFI_CNTRL_DFI_CKE1_SHIFT   4
#define PHY_CONTROL_DFI_CNTRL_DFI_CKE1_MASK    (0x1<<PHY_CONTROL_DFI_CNTRL_DFI_CKE1_SHIFT)
#define PHY_CONTROL_DFI_CNTRL_DFI_CKE0_SHIFT   3
#define PHY_CONTROL_DFI_CNTRL_DFI_CKE0_MASK    (0x1<<PHY_CONTROL_DFI_CNTRL_DFI_CKE0_SHIFT)
#define PHY_CONTROL_DFI_CNTRL_ASSERT_REQ_SHIFT 0
#define PHY_CONTROL_DFI_CNTRL_ASSERT_REQ_MASK  (0x1<<PHY_CONTROL_DFI_CNTRL_ASSERT_REQ_SHIFT)
	uint32_t WRITE_ODT_CTRL;	/* 0x240 */
	uint32_t ABI_PAR_CTRL;	/* 0x244 */
	uint32_t ZQ_CAL;	/* 0x248 */
	uint32_t unused5[109];	/* 0x24c-0x3ff */
} DDRPhyControl;

typedef struct DDRPhyByteLaneControl {
	uint32_t VDL_CTRL_WR[12];	/* 0x00 - 0x2c */
	uint32_t VDL_CTRL_RD[24];	/* 0x30 - 0x8c */
	uint32_t VDL_CLK_CTRL;	/* 0x90 */
	uint32_t VDL_LDE_CTRL;	/* 0x94 */
	uint32_t unused0[2];	/* 0x98-0x9f */
	uint32_t RD_EN_DLY_CYC;	/* 0xa0 */
	uint32_t WR_CHAN_DLY_CYC;	/* 0xa4 */
	uint32_t unused1[2];	/* 0xa8-0xaf */
	uint32_t RD_CTRL;	/* 0xb0 */
	uint32_t RD_FIFO_ADDR;	/* 0xb4 */
	uint32_t RD_FIFO_DATA;	/* 0xb8 */
	uint32_t RD_FIFO_DM_DBI;	/* 0xbc */
	uint32_t RD_FIFO_STATUS;	/* 0xc0 */
	uint32_t RD_FIFO_CLR;	/* 0xc4 */
	uint32_t IDLE_PAD_CTRL;	/* 0xc8 */
	uint32_t DRIVE_PAD_CTRL;	/* 0xcc */
	uint32_t RD_EN_DRIVE_PAD_CTRL;	/* 0xd0 */
	uint32_t STATIC_PAD_CTRL;	/* 0xd4 */
	uint32_t WR_PREAMBLE_MODE;	/* 0xd8 */
	uint32_t unused2;	/* 0xdc */
	uint32_t ODT_CTRL;	/* 0xe0 */
	uint32_t unused3[3];	/* 0xe4-0xef */
	uint32_t EDC_DPD_CTRL;	/* 0xf0 */
	uint32_t EDC_DPD_STATUS;	/* 0xf4 */
	uint32_t EDC_DPD_OUT_CTRL;	/* 0xf8 */
	uint32_t EDC_DPD_OUT_STATUS;	/* 0xfc */
	uint32_t EDC_DPD_OUT_STATUS_CLEAR;	/* 0x100 */
	uint32_t EDC_CRC_CTRL;	/* 0x104 */
	uint32_t EDC_CRC_STATUS;	/* 0x108 */
	uint32_t EDC_CRC_COUNT;	/* 0x10c */
	uint32_t EDC_CRC_STATUS_CLEAR;	/* 0x110 */
	uint32_t BL_SPARE_REG;	/* 0x114 */
	uint32_t unused4[58];
} DDRPhyByteLaneControl;

typedef struct MEMCControl {
	uint32_t GLB_VERS;	/* 0x000 */
	uint32_t GLB_GCFG;	/* 0x004 */
#define MEMC_GLB_GCFG_DRAM_EN_SHIFT        31
#define MEMC_GLB_GCFG_DRAM_EN_MASK         (0x1<<MEMC_GLB_GCFG_DRAM_EN_SHIFT)
#define MEMC_GLB_GCFG_PHY_4X_SHIFT         24
#define MEMC_GLB_GCFG_PHY_4X_MASK          (0x1<<MEMC_GLB_GCFG_PHY_4X_SHIFT)
#define MEMC_GLB_GCFG_MCLKSRC_SHIFT        9
#define MEMC_GLB_GCFG_MCLKSRC_MASK         (0x1<<MEMC_GLB_GCFG_MCLKSRC_SHIFT)
#define MEMC_GLB_GCFG_MEMINITDONE_SHIFT    8
#define MEMC_GLB_GCFG_MEMINITDONE_MASK     (0x1<<MEMC_GLB_GCFG_MEMINITDONE_SHIFT)
#define MEMC_GLB_GCFG_SIZE1_SHIFT          0
#define MEMC_GLB_GCFG_SIZE1_MASK           (0xf<<MEMC_GLB_GCFG_SIZE1_SHIFT)

	uint32_t unused0[2];	/* 0x008-0x00f */
	uint32_t GLB_CFG;	/* 0x010 */
#define MEMC_GLB_CFG_RR_MODE_SHIFT      0x0
#define MEMC_GLB_CFG_RR_MODE_MASK       (0x1<<MEMC_GLB_CFG_RR_MODE_SHIFT)
#define MEMC_GLB_CFG_BURST_MODE_SHIFT   0x1
#define MEMC_GLB_CFG_BURST_MODE_MASK    (0x1<<MEMC_GLB_CFG_BURST_MODE_SHIFT)
	uint32_t GLB_QUE_DIS;	/* 0x014 */
	uint32_t GLB_SP_SEL;	/* 0x018 */
#define MEMC_GLB_SP_SEL_SELECT_MASK     0x001fffff
	uint32_t GLB_SP_PRI_0;	/* 0x01c */
	uint32_t GLB_SP_PRI_1;	/* 0x020 */
	uint32_t GLB_SP_PRI_2;	/* 0x024 */
	uint32_t GLB_SP_PRI_3;	/* 0x028 */
	uint32_t GLB_SP_PRI_4;	/* 0x02c */
	uint32_t GLB_SP_PRI_5;	/* 0x030 */
	uint32_t unused1[3];	/* 0x034-0x3f */
	uint32_t GLB_RR_QUANTUM[12];	/* 0x040-0x06c */
	uint32_t unused2[4];	/* 0x070-0x07f */

	uint32_t INTR2_CPU_STATUS;	/* 0x080 */
	uint32_t INTR2_CPU_SET;	/* 0x084 */
	uint32_t INTR2_CPU_CLEAR;	/* 0x088 */
	uint32_t INTR2_CPU_MASK_STATUS;	/* 0x08c */
	uint32_t INTR2_CPU_MASK_SET;	/* 0x090 */
	uint32_t INTR2_CPU_MASK_CLEAR;	/* 0x094 */
	uint32_t unused3[10];	/* 0x098-0x0bf */

	uint32_t SRAM_REMAP_CTRL;	/* 0x0c0 */
	uint32_t SRAM_REMAP_INIT;	/* 0x0c4 */
	uint32_t SRAM_REMAP_LOG_INFO_0;	/* 0x0c8 */
	uint32_t SRAM_REMAP_LOG_INFO_1;	/* 0x0cc */
	uint32_t unused4[12];	/* 0x0d0-0x0ff */

	uint32_t CHN_CFG_CNFG;	/* 0x100 */
	uint32_t CHN_CFG_CSST;	/* 0x104 */
	uint32_t CHN_CFG_CSEND;	/* 0x108 */
	uint32_t unused5;	/* 0x10c */
	uint32_t CHN_CFG_ROW00_0;	/* 0x110 */
	uint32_t CHN_CFG_ROW00_1;	/* 0x114 */
	uint32_t CHN_CFG_ROW01_0;	/* 0x118 */
	uint32_t CHN_CFG_ROW01_1;	/* 0x11c */
	uint32_t unused6[4];	/* 0x120-0x12f */
	uint32_t CHN_CFG_ROW20_0;	/* 0x130 */
	uint32_t CHN_CFG_ROW20_1;	/* 0x134 */
	uint32_t CHN_CFG_ROW21_0;	/* 0x138 */
	uint32_t CHN_CFG_ROW21_1;	/* 0x13c */
	uint32_t unused7[4];	/* 0x140-0x14f */
	uint32_t CHN_CFG_COL00_0;	/* 0x150 */
	uint32_t CHN_CFG_COL00_1;	/* 0x154 */
	uint32_t CHN_CFG_COL01_0;	/* 0x158 */
	uint32_t CHN_CFG_COL01_1;	/* 0x15c */
	uint32_t unused8[4];	/* 0x160-0x16f */
	uint32_t CHN_CFG_COL20_0;	/* 0x170 */
	uint32_t CHN_CFG_COL20_1;	/* 0x174 */
	uint32_t CHN_CFG_COL21_0;	/* 0x178 */
	uint32_t CHN_CFG_COL21_1;	/* 0x17c */
	uint32_t unused9[4];	/* 0x180-0x18f */
	uint32_t CHN_CFG_BNK10;	/* 0x190 */
	uint32_t CHN_CFG_BNK32;	/* 0x194 */
	uint32_t unused10[26];	/* 0x198-0x1ff */

	uint32_t CHN_TIM_DCMD;	/* 0x200 */
	uint32_t CHN_TIM_DMODE_0;	/* 0x204 */
	uint32_t CHN_TIM_DMODE_2;	/* 0x208 */
	uint32_t CHN_TIM_CLKS;	/* 0x20c */
	uint32_t CHN_TIM_ODT;	/* 0x210 */
	uint32_t CHN_TIM_TIM1_0;	/* 0x214 */
	uint32_t CHN_TIM_TIM1_1;	/* 0x218 */
	uint32_t CHN_TIM_TIM2;	/* 0x21c */
	uint32_t CHN_TIM_CTL_CRC;	/* 0x220 */
	uint32_t CHN_TIM_DOUT_CRC;	/* 0x224 */
	uint32_t CHN_TIM_DIN_CRC;	/* 0x228 */
	uint32_t CHN_TIM_CRC_CTRL;	/* 0x22c */
	uint32_t CHN_TIM_PHY_ST;	/* 0x230 */
#define MEMC_CHN_TIM_PHY_ST_PHY_ST_POWER_UP       0x1
#define MEMC_CHN_TIM_PHY_ST_PHY_ST_HW_RESET       0x2
#define MEMC_CHN_TIM_PHY_ST_PHY_ST_SW_RESET       0x4
#define MEMC_CHN_TIM_PHY_ST_PHY_ST_READY          0x10
	uint32_t CHN_TIM_DRAM_CFG;	/* 0x234 */
#define DRAM_CFG_DRAMSLEEP          (1<<11)
#define MEMC_CHN_TIM_DRAM_CFG_DRAM_CFG_DRAMTYPE_SHIFT 0x0
#define MEMC_CHN_TIM_DRAM_CFG_DRAM_CFG_DRAMTYPE_MASK  (0xf<<MEMC_CHN_TIM_DRAM_CFG_DRAM_CFG_DRAMTYPE_SHIFT)
	uint32_t CHN_TIM_STAT;	/* 0x238 */
	uint32_t unused11[49];	/* 0x23c-0x2ff */

	UBUSInterface UBUSIF0;	/* 0x300-0x33f */
	UBUSInterface UBUSIF1;	/* 0x340-0x37f */

	uint32_t AXIRIF_0_CFG;	/* 0x380 */
	uint32_t AXIRIF_0_REP_ARB_MODE;	/* 0x384 */
#define MEMC_AXIRIF_0_REP_ARB_MODE_FIFO_MODE_SHIFT    0x0
#define MEMC_AXIRIF_0_REP_ARB_MODE_FIFO_MODE_MASK     (0x3<<MEMC_AXIRIF_0_REP_ARB_MODE_FIFO_MODE_SHIFT)
	uint32_t AXIRIF_0_SCRATCH;	/* 0x388 */
	uint32_t unused12[29];	/* 0x38c-0x3ff */

	uint32_t AXIWIF_0_CFG;	/* 0x400 */
#define MEMC_AXIWIF_0_CFG_WRITE_REPLY_MODE_SHIFT      0x0
#define MEMC_AXIWIF_0_CFG_WRITE_REPLY_MODE_MASK       (0x1<<MEMC_AXIWIF_0_CFG_WRITE_REPLY_MODE_SHIFT)
#define MEMC_AXIWIF_0_CFG_WRITE_BURST_MODE_SHIFT      0x1
#define MEMC_AXIWIF_0_CFG_WRITE_BURST_MODE_MASK       (0x1<<MEMC_AXIWIF_0_CFG_WRITE_BURST_MODE_SHIFT)
	uint32_t AXIWIF_0_REP_ARB_MODE;	/* 0x404 */
#define MEMC_AXIWIF_0_REP_ARB_MODE_FIFO_MODE_SHIFT    0x0
#define MEMC_AXIWIF_0_REP_ARB_MODE_FIFO_MODE_MASK     (0x1<<MEMC_AXIRIF_0_REP_ARB_MODE_FIFO_MODE_SHIFT)

	uint32_t AXIWIF_0_SCRATCH;	/* 0x408 */
	uint32_t unused13[61];	/* 0x40c-0x4ff */

	EDISEngine EDIS_0;	/* 0x500 */
	EDISEngine EDIS_1;	/* 0x600 */

	uint32_t STATS_CTRL;	/* 0x700 */
	uint32_t STATS_TIMER_CFG;	/* 0x704 */
	uint32_t STATS_TIMER_COUNT;	/* 0x708 */
	uint32_t STATS_TOTAL_SLICE;	/* 0x70c */
	uint32_t STATS_TOTAL_PACKET;	/* 0x710 */
	uint32_t STATS_SLICE_REORDER;	/* 0x714 */
	uint32_t STATS_IDLE_DDR_CYCLE;	/* 0x718 */
	uint32_t STATS_ARB_GRANT;	/* 0x71c */
	uint32_t STATS_PROG_0;	/* 0x720 */
	uint32_t STATS_PROG_1;	/* 0x724 */
	uint32_t STATS_ARB_GRANT_MATCH;	/* 0x728 */
	uint32_t STATS_CFG_0;	/* 0x72c */
	uint32_t STATS_CFG_1;	/* 0x730 */
	uint32_t unused14[19];	/* 0x734-0x77f */

	uint32_t CAP_CAPTURE_CFG;	/* 0x780 */
	uint32_t CAP_TRIGGER_ADDR;	/* 0x784 */
	uint32_t CAP_READ_CTRL;	/* 0x788 */
	uint32_t unused15;	/* 0x78c */
	uint32_t CAP_CAPTURE_MATCH0;	/* 0x790 */
	uint32_t CAP_CAPTURE_MATCH1;	/* 0x794 */
	uint32_t CAP_CAPTURE_MATCH2;	/* 0x798 */
	uint32_t unused16;	/* 0x79c */
	uint32_t CAP_CAPTURE_MASK0;	/* 0x7a0 */
	uint32_t CAP_CAPTURE_MASK1;	/* 0x7a4 */
	uint32_t CAP_CAPTURE_MASK2;	/* 0x7a8 */
	uint32_t unused17;	/* 0x7ac */
	uint32_t CAP_TRIGGER_MATCH0;	/* 0x7b0 */
	uint32_t CAP_TRIGGER_MATCH1;	/* 0x7b4 */
	uint32_t CAP_TRIGGER_MATCH2;	/* 0x7b8 */
	uint32_t unused18;	/* 0x7bc */
	uint32_t CAP_TRIGGER_MASK0;	/* 0x7c0 */
	uint32_t CAP_TRIGGER_MASK1;	/* 0x7c4 */
	uint32_t CAP_TRIGGER_MASK2;	/* 0x7c8 */
	uint32_t unused19;	/* 0x7cc */
	uint32_t CAP_READ_DATA[4];	/* 0x7d0-0x7dc */
	uint32_t unused20[8];	/* 0x7e0-0x7ff */

	SecureRangeCheckers SEC_RANGE_CHK;	/* 0x800 */

	uint32_t SEC_INTR2_CPU_STATUS;	/* 0x900 */
	uint32_t SEC_INTR2_CPU_SET;	/* 0x904 */
	uint32_t SEC_INTR2_CPU_CLEAR;	/* 0x908 */
	uint32_t SEC_INTR2_CPU_MASK_STATUS;	/* 0x90c */
	uint32_t SEC_INTR2_CPU_MASK_SET;	/* 0x910 */
	uint32_t SEC_INTR2_CPU_MASK_CLEAR;	/* 0x914 */
	uint32_t unused21[10];	/* 0x918-0x93f */

	uint32_t SEC_SRAM_REMAP_CTRL;	/* 0x940 */
	uint32_t SEC_SRAM_REMAP_INIT;	/* 0x944 */
	uint32_t SEC_SRAM_REMAP_LOG_0;	/* 0x948 */
	uint32_t SEC_SRAM_REMAP_LOG_1;	/* 0x94c */
	uint32_t unused22[428];	/* 0x950-0xfff */

	DDRPhyControl PhyControl;	/* 0x1000 */
	DDRPhyByteLaneControl PhyByteLane0Control;	/* 0x1400 */
	DDRPhyByteLaneControl PhyByteLane1Control;	/* 0x1600 */
} MEMCControl;

#define MEMC ((volatile MEMCControl * const) MEMC_BASE)

#endif
