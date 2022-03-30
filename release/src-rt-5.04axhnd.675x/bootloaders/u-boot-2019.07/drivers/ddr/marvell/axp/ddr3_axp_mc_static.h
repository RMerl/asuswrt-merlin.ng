/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 */

#ifndef __AXP_MC_STATIC_H
#define __AXP_MC_STATIC_H

MV_DRAM_MC_INIT ddr3_A0_db_667[MV_MAX_DDR3_STATIC_SIZE] = {
#ifdef CONFIG_DDR_32BIT
	{0x00001400, 0x7301c924},	/*DDR SDRAM Configuration Register */
#else /*CONFIG_DDR_64BIT */
	{0x00001400, 0x7301CA28},	/*DDR SDRAM Configuration Register */
#endif
	{0x00001404, 0x3630b800},	/*Dunit Control Low Register */
	{0x00001408, 0x43149775},	/*DDR SDRAM Timing (Low) Register */
	/* {0x0000140C, 0x38000C6A}, *//*DDR SDRAM Timing (High) Register */
	{0x0000140C, 0x38d83fe0},	/*DDR SDRAM Timing (High) Register */

#ifdef DB_78X60_PCAC
	{0x00001410, 0x040F0001},	/*DDR SDRAM Address Control Register */
#else
	{0x00001410, 0x040F0000},	/*DDR SDRAM Open Pages Control Register */
#endif

	{0x00001414, 0x00000000},	/*DDR SDRAM Open Pages Control Register */
	{0x00001418, 0x00000e00},	/*DDR SDRAM Operation Register */
	{0x00001420, 0x00000004},	/*DDR SDRAM Extended Mode Register */
	{0x00001424, 0x0000D3FF},	/*Dunit Control High Register */
	{0x00001428, 0x000F8830},	/*Dunit Control High Register */
	{0x0000142C, 0x214C2F38},	/*Dunit Control High Register */
	{0x0000147C, 0x0000c671},

	{0x000014a0, 0x000002A9},
	{0x000014a8, 0x00000101},	/*2:1 */
	{0x00020220, 0x00000007},

	{0x00001494, 0x00010000},	/*DDR SDRAM ODT Control (Low) Register */
	{0x00001498, 0x00000000},	/*DDR SDRAM ODT Control (High) Register */
	{0x0000149C, 0x00000301},	/*DDR Dunit ODT Control Register */

	{0x000014C0, 0x192434e9},	/* DRAM address and Control Driving Strenght  */
	{0x000014C4, 0x092434e9},	/* DRAM Data and DQS Driving Strenght  */

	{0x000200e8, 0x3FFF0E01},	/* DO NOT Modify - Open Mbus Window - 2G - Mbus is required for the training sequence */
	{0x00020184, 0x3FFFFFE0},	/* DO NOT Modify - Close fast path Window to - 2G */

	{0x0001504, 0x7FFFFFF1},	/* CS0 Size */
	{0x000150C, 0x00000000},	/* CS1 Size */
	{0x0001514, 0x00000000},	/* CS2 Size */
	{0x000151C, 0x00000000},	/* CS3 Size */

	/*     {0x00001524, 0x0000C800},  */
	{0x00001538, 0x0000000b},	/*Read Data Sample Delays Register */
	{0x0000153C, 0x0000000d},	/*Read Data Ready Delay Register */

	{0x000015D0, 0x00000640},	/*MR0 */
	{0x000015D4, 0x00000046},	/*MR1 */
	{0x000015D8, 0x00000010},	/*MR2 */
	{0x000015DC, 0x00000000},	/*MR3 */

	{0x000015E4, 0x00203c18},	/*ZQC Configuration Register */
	{0x000015EC, 0xd800aa25},	/*DDR PHY */
	{0x0, 0x0}
};

MV_DRAM_MC_INIT ddr3_A0_AMC_667[MV_MAX_DDR3_STATIC_SIZE] = {
#ifdef CONFIG_DDR_32BIT
	{0x00001400, 0x7301c924},	/*DDR SDRAM Configuration Register */
#else /*CONFIG_DDR_64BIT */
	{0x00001400, 0x7301CA28},	/*DDR SDRAM Configuration Register */
#endif
	{0x00001404, 0x3630b800},	/*Dunit Control Low Register */
	{0x00001408, 0x43149775},	/*DDR SDRAM Timing (Low) Register */
	/* {0x0000140C, 0x38000C6A}, *//*DDR SDRAM Timing (High) Register */
	{0x0000140C, 0x38d83fe0},	/*DDR SDRAM Timing (High) Register */

#ifdef DB_78X60_PCAC
	{0x00001410, 0x040F0001},	/*DDR SDRAM Address Control Register */
#else
	{0x00001410, 0x040F000C},	/*DDR SDRAM Open Pages Control Register */
#endif

	{0x00001414, 0x00000000},	/*DDR SDRAM Open Pages Control Register */
	{0x00001418, 0x00000e00},	/*DDR SDRAM Operation Register */
	{0x00001420, 0x00000004},	/*DDR SDRAM Extended Mode Register */
	{0x00001424, 0x0000D3FF},	/*Dunit Control High Register */
	{0x00001428, 0x000F8830},	/*Dunit Control High Register */
	{0x0000142C, 0x214C2F38},	/*Dunit Control High Register */
	{0x0000147C, 0x0000c671},

	{0x000014a0, 0x000002A9},
	{0x000014a8, 0x00000101},	/*2:1 */
	{0x00020220, 0x00000007},

	{0x00001494, 0x00010000},	/*DDR SDRAM ODT Control (Low) Register */
	{0x00001498, 0x00000000},	/*DDR SDRAM ODT Control (High) Register */
	{0x0000149C, 0x00000301},	/*DDR Dunit ODT Control Register */

	{0x000014C0, 0x192434e9},	/* DRAM address and Control Driving Strenght  */
	{0x000014C4, 0x092434e9},	/* DRAM Data and DQS Driving Strenght  */

	{0x000200e8, 0x3FFF0E01},	/* DO NOT Modify - Open Mbus Window - 2G - Mbus is required for the training sequence */
	{0x00020184, 0x3FFFFFE0},	/* DO NOT Modify - Close fast path Window to - 2G */

	{0x0001504, 0x3FFFFFF1},	/* CS0 Size */
	{0x000150C, 0x00000000},	/* CS1 Size */
	{0x0001514, 0x00000000},	/* CS2 Size */
	{0x000151C, 0x00000000},	/* CS3 Size */

	/*     {0x00001524, 0x0000C800},  */
	{0x00001538, 0x0000000b},	/*Read Data Sample Delays Register */
	{0x0000153C, 0x0000000d},	/*Read Data Ready Delay Register */

	{0x000015D0, 0x00000640},	/*MR0 */
	{0x000015D4, 0x00000046},	/*MR1 */
	{0x000015D8, 0x00000010},	/*MR2 */
	{0x000015DC, 0x00000000},	/*MR3 */

	{0x000015E4, 0x00203c18},	/*ZQC Configuration Register */
	{0x000015EC, 0xd800aa25},	/*DDR PHY */
	{0x0, 0x0}
};

MV_DRAM_MC_INIT ddr3_A0_db_400[MV_MAX_DDR3_STATIC_SIZE] = {
#ifdef CONFIG_DDR_32BIT
	{0x00001400, 0x73004C30},	/*DDR SDRAM Configuration Register */
#else /* CONFIG_DDR_64BIT */
	{0x00001400, 0x7300CC30},	/*DDR SDRAM Configuration Register */
#endif
	{0x00001404, 0x3630B840},	/*Dunit Control Low Register */
	{0x00001408, 0x33137663},	/*DDR SDRAM Timing (Low) Register */
	{0x0000140C, 0x38000C55},	/*DDR SDRAM Timing (High) Register */
	{0x00001410, 0x040F0000},	/*DDR SDRAM Address Control Register */
	{0x00001414, 0x00000000},	/*DDR SDRAM Open Pages Control Register */
	{0x00001418, 0x00000e00},	/*DDR SDRAM Operation Register */
	{0x0000141C, 0x00000672},	/*DDR SDRAM Mode Register */
	{0x00001420, 0x00000004},	/*DDR SDRAM Extended Mode Register */
	{0x00001424, 0x0100D3FF},	/*Dunit Control High Register */
	{0x00001428, 0x000D6720},	/*Dunit Control High Register */
	{0x0000142C, 0x014C2F38},	/*Dunit Control High Register */
	{0x0000147C, 0x00006571},

	{0x00001494, 0x00010000},	/*DDR SDRAM ODT Control (Low) Register */
	{0x00001498, 0x00000000},	/*DDR SDRAM ODT Control (High) Register */
	{0x0000149C, 0x00000301},	/*DDR Dunit ODT Control Register */

	{0x000014a0, 0x000002A9},
	{0x000014a8, 0x00000101},	/*2:1 */
	{0x00020220, 0x00000007},

	{0x000014C0, 0x192424C8},	/* DRAM address and Control Driving Strenght  */
	{0x000014C4, 0xEFB24C8},	/* DRAM Data and DQS Driving Strenght  */

	{0x000200e8, 0x3FFF0E01},	/* DO NOT Modify - Open Mbus Window - 2G - Mbus is required for the training sequence */
	{0x00020184, 0x3FFFFFE0},	/* DO NOT Modify - Close fast path Window to - 2G */

	{0x0001504, 0x7FFFFFF1},	/* CS0 Size */
	{0x000150C, 0x00000000},	/* CS1 Size */
	{0x0001514, 0x00000000},	/* CS2 Size */
	{0x000151C, 0x00000000},	/* CS3 Size */

	{0x00001538, 0x00000008},	/*Read Data Sample Delays Register */
	{0x0000153C, 0x0000000A},	/*Read Data Ready Delay Register */

	{0x000015D0, 0x00000630},	/*MR0 */
	{0x000015D4, 0x00000046},	/*MR1 */
	{0x000015D8, 0x00000008},	/*MR2 */
	{0x000015DC, 0x00000000},	/*MR3 */

	{0x000015E4, 0x00203c18},	/*ZQDS Configuration Register */
	/* {0x000015EC, 0xDE000025}, *//*DDR PHY */
	{0x000015EC, 0xF800AA25},	/*DDR PHY */
	{0x0, 0x0}
};

MV_DRAM_MC_INIT ddr3_Z1_db_600[MV_MAX_DDR3_STATIC_SIZE] = {
#ifdef CONFIG_DDR_32BIT
	{0x00001400, 0x73014A28},	/*DDR SDRAM Configuration Register */
#else /*CONFIG_DDR_64BIT */
	{0x00001400, 0x7301CA28},	/*DDR SDRAM Configuration Register */
#endif
	{0x00001404, 0x3630B040},	/*Dunit Control Low Register */
	{0x00001408, 0x44149887},	/*DDR SDRAM Timing (Low) Register */
	/* {0x0000140C, 0x38000C6A}, *//*DDR SDRAM Timing (High) Register */
	{0x0000140C, 0x38D83FE0},	/*DDR SDRAM Timing (High) Register */

#ifdef DB_78X60_PCAC
	{0x00001410, 0x040F0001},	/*DDR SDRAM Address Control Register */
#else
	{0x00001410, 0x040F0000},	/*DDR SDRAM Open Pages Control Register */
#endif

	{0x00001414, 0x00000000},	/*DDR SDRAM Open Pages Control Register */
	{0x00001418, 0x00000e00},	/*DDR SDRAM Operation Register */
	{0x00001420, 0x00000004},	/*DDR SDRAM Extended Mode Register */
	{0x00001424, 0x0100D1FF},	/*Dunit Control High Register */
	{0x00001428, 0x000F8830},	/*Dunit Control High Register */
	{0x0000142C, 0x214C2F38},	/*Dunit Control High Register */
	{0x0000147C, 0x0000c671},

	{0x000014a8, 0x00000101},	/*2:1 */
	{0x00020220, 0x00000007},

	{0x00001494, 0x00010000},	/*DDR SDRAM ODT Control (Low) Register */
	{0x00001498, 0x00000000},	/*DDR SDRAM ODT Control (High) Register */
	{0x0000149C, 0x00000301},	/*DDR Dunit ODT Control Register */

	{0x000014C0, 0x192424C8},	/* DRAM address and Control Driving Strenght  */
	{0x000014C4, 0xEFB24C8},	/* DRAM Data and DQS Driving Strenght  */

	{0x000200e8, 0x3FFF0E01},	/* DO NOT Modify - Open Mbus Window - 2G - Mbus is required for the training sequence */
	{0x00020184, 0x3FFFFFE0},	/* DO NOT Modify - Close fast path Window to - 2G */

	{0x0001504, 0x7FFFFFF1},	/* CS0 Size */
	{0x000150C, 0x00000000},	/* CS1 Size */
	{0x0001514, 0x00000000},	/* CS2 Size */
	{0x000151C, 0x00000000},	/* CS3 Size */

	/*     {0x00001524, 0x0000C800},  */
	{0x00001538, 0x0000000b},	/*Read Data Sample Delays Register */
	{0x0000153C, 0x0000000d},	/*Read Data Ready Delay Register */

	{0x000015D0, 0x00000650},	/*MR0 */
	{0x000015D4, 0x00000046},	/*MR1 */
	{0x000015D8, 0x00000010},	/*MR2 */
	{0x000015DC, 0x00000000},	/*MR3 */

	{0x000015E4, 0x00203c18},	/*ZQC Configuration Register */
	{0x000015EC, 0xDE000025},	/*DDR PHY */
	{0x0, 0x0}
};

MV_DRAM_MC_INIT ddr3_Z1_db_300[MV_MAX_DDR3_STATIC_SIZE] = {
#ifdef CONFIG_DDR_32BIT
	{0x00001400, 0x73004C30},	/*DDR SDRAM Configuration Register */
#else /*CONFIG_DDR_64BIT */
	{0x00001400, 0x7300CC30},	/*DDR SDRAM Configuration Register */
	/*{0x00001400, 0x7304CC30},  *//*DDR SDRAM Configuration Register */
#endif
	{0x00001404, 0x3630B840},	/*Dunit Control Low Register */
	{0x00001408, 0x33137663},	/*DDR SDRAM Timing (Low) Register */
	{0x0000140C, 0x38000C55},	/*DDR SDRAM Timing (High) Register */
	{0x00001410, 0x040F0000},	/*DDR SDRAM Address Control Register */
	{0x00001414, 0x00000000},	/*DDR SDRAM Open Pages Control Register */
	{0x00001418, 0x00000e00},	/*DDR SDRAM Operation Register */
	{0x0000141C, 0x00000672},	/*DDR SDRAM Mode Register */
	{0x00001420, 0x00000004},	/*DDR SDRAM Extended Mode Register */
	{0x00001424, 0x0100F1FF},	/*Dunit Control High Register */
	{0x00001428, 0x000D6720},	/*Dunit Control High Register */
	{0x0000142C, 0x014C2F38},	/*Dunit Control High Register */
	{0x0000147C, 0x00006571},

	{0x00001494, 0x00010000},	/*DDR SDRAM ODT Control (Low) Register */
	{0x00001498, 0x00000000},	/*DDR SDRAM ODT Control (High) Register */
	{0x0000149C, 0x00000301},	/*DDR Dunit ODT Control Register */

	{0x000014C0, 0x192424C8},	/* DRAM address and Control Driving Strenght  */
	{0x000014C4, 0xEFB24C8},	/* DRAM Data and DQS Driving Strenght  */

	{0x000200e8, 0x3FFF0E01},	/* DO NOT Modify - Open Mbus Window - 2G - Mbus is required for the training sequence */
	{0x00020184, 0x3FFFFFE0},	/* DO NOT Modify - Close fast path Window to - 2G */

	{0x0001504, 0x7FFFFFF1},	/* CS0 Size */
	{0x000150C, 0x00000000},	/* CS1 Size */
	{0x0001514, 0x00000000},	/* CS2 Size */
	{0x000151C, 0x00000000},	/* CS3 Size */

	{0x00001538, 0x00000008},	/*Read Data Sample Delays Register */
	{0x0000153C, 0x0000000A},	/*Read Data Ready Delay Register */

	{0x000015D0, 0x00000630},	/*MR0 */
	{0x000015D4, 0x00000046},	/*MR1 */
	{0x000015D8, 0x00000008},	/*MR2 */
	{0x000015DC, 0x00000000},	/*MR3 */

	{0x000015E4, 0x00203c18},	/*ZQDS Configuration Register */
	{0x000015EC, 0xDE000025},	/*DDR PHY */

	{0x0, 0x0}
};

#endif /* __AXP_MC_STATIC_H */
