/* SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause */
/*
 * Copyright (C) 2018, STMicroelectronics - All Rights Reserved
 */

#ifndef _RAM_STM32MP1_DDR_REGS_H
#define _RAM_STM32MP1_DDR_REGS_H

#include <linux/bitops.h>

/* DDR3/LPDDR2/LPDDR3 Controller (DDRCTRL) registers */
struct stm32mp1_ddrctl {
	u32 mstr ;		/* 0x0 Master*/
	u32 stat;		/* 0x4 Operating Mode Status*/
	u8 reserved008[0x10 - 0x8];
	u32 mrctrl0;		/* 0x10 Control 0.*/
	u32 mrctrl1;		/* 0x14 Control 1*/
	u32 mrstat;		/* 0x18 Status*/
	u32 reserved01c;	/* 0x1c */
	u32 derateen;		/* 0x20 Temperature Derate Enable*/
	u32 derateint;		/* 0x24 Temperature Derate Interval*/
	u8 reserved028[0x30 - 0x28];
	u32 pwrctl;		/* 0x30 Low Power Control*/
	u32 pwrtmg;		/* 0x34 Low Power Timing*/
	u32 hwlpctl;		/* 0x38 Hardware Low Power Control*/
	u8 reserved03c[0x50 - 0x3C];
	u32 rfshctl0;		/* 0x50 Refresh Control 0*/
	u32 reserved054;	/* 0x54 Refresh Control 1*/
	u32 reserved058;	/* 0x58 Refresh Control 2*/
	u32 reserved05C;
	u32 rfshctl3;		/* 0x60 Refresh Control 0*/
	u32 rfshtmg;		/* 0x64 Refresh Timing*/
	u8 reserved068[0xc0 - 0x68];
	u32 crcparctl0;		/* 0xc0 CRC Parity Control0*/
	u32 reserved0c4;	/* 0xc4 CRC Parity Control1*/
	u32 reserved0c8;	/* 0xc8 CRC Parity Control2*/
	u32 crcparstat;		/* 0xcc CRC Parity Status*/
	u32 init0;		/* 0xd0 SDRAM Initialization 0*/
	u32 init1;		/* 0xd4 SDRAM Initialization 1*/
	u32 init2;		/* 0xd8 SDRAM Initialization 2*/
	u32 init3;		/* 0xdc SDRAM Initialization 3*/
	u32 init4;		/* 0xe0 SDRAM Initialization 4*/
	u32 init5;		/* 0xe4 SDRAM Initialization 5*/
	u32 reserved0e8;
	u32 reserved0ec;
	u32 dimmctl;		/* 0xf0 DIMM Control*/
	u8 reserved0f4[0x100 - 0xf4];
	u32 dramtmg0;		/* 0x100 SDRAM Timing 0*/
	u32 dramtmg1;		/* 0x104 SDRAM Timing 1*/
	u32 dramtmg2;		/* 0x108 SDRAM Timing 2*/
	u32 dramtmg3;		/* 0x10c SDRAM Timing 3*/
	u32 dramtmg4;		/* 0x110 SDRAM Timing 4*/
	u32 dramtmg5;		/* 0x114 SDRAM Timing 5*/
	u32 dramtmg6;		/* 0x118 SDRAM Timing 6*/
	u32 dramtmg7;		/* 0x11c SDRAM Timing 7*/
	u32 dramtmg8;		/* 0x120 SDRAM Timing 8*/
	u8 reserved124[0x138 - 0x124];
	u32 dramtmg14;		/* 0x138 SDRAM Timing 14*/
	u32 dramtmg15;		/* 0x13C SDRAM Timing 15*/
	u8 reserved140[0x180 - 0x140];
	u32 zqctl0;		/* 0x180 ZQ Control 0*/
	u32 zqctl1;		/* 0x184 ZQ Control 1*/
	u32 zqctl2;		/* 0x188 ZQ Control 2*/
	u32 zqstat;		/* 0x18c ZQ Status*/
	u32 dfitmg0;		/* 0x190 DFI Timing 0*/
	u32 dfitmg1;		/* 0x194 DFI Timing 1*/
	u32 dfilpcfg0;		/* 0x198 DFI Low Power Configuration 0*/
	u32 reserved19c;
	u32 dfiupd0;		/* 0x1a0 DFI Update 0*/
	u32 dfiupd1;		/* 0x1a4 DFI Update 1*/
	u32 dfiupd2;		/* 0x1a8 DFI Update 2*/
	u32 reserved1ac;
	u32 dfimisc;		/* 0x1b0 DFI Miscellaneous Control*/
	u8 reserved1b4[0x1bc - 0x1b4];
	u32 dfistat;		/* 0x1bc DFI Miscellaneous Control*/
	u8 reserved1c0[0x1c4 - 0x1c0];
	u32 dfiphymstr;		/* 0x1c4 DFI PHY Master interface*/
	u8 reserved1c8[0x204 - 0x1c8];
	u32 addrmap1;		/* 0x204 Address Map 1*/
	u32 addrmap2;		/* 0x208 Address Map 2*/
	u32 addrmap3;		/* 0x20c Address Map 3*/
	u32 addrmap4;		/* 0x210 Address Map 4*/
	u32 addrmap5;		/* 0x214 Address Map 5*/
	u32 addrmap6;		/* 0x218 Address Map 6*/
	u8 reserved21c[0x224 - 0x21c];
	u32 addrmap9;		/* 0x224 Address Map 9*/
	u32 addrmap10;		/* 0x228 Address Map 10*/
	u32 addrmap11;		/* 0x22C Address Map 11*/
	u8 reserved230[0x240 - 0x230];
	u32 odtcfg;		/* 0x240 ODT Configuration*/
	u32 odtmap;		/* 0x244 ODT/Rank Map*/
	u8 reserved248[0x250 - 0x248];
	u32 sched;		/* 0x250 Scheduler Control*/
	u32 sched1;		/* 0x254 Scheduler Control 1*/
	u32 reserved258;
	u32 perfhpr1;		/* 0x25c High Priority Read CAM 1*/
	u32 reserved260;
	u32 perflpr1;		/* 0x264 Low Priority Read CAM 1*/
	u32 reserved268;
	u32 perfwr1;		/* 0x26c Write CAM 1*/
	u8 reserved27c[0x300 - 0x270];
	u32 dbg0;		/* 0x300 Debug 0*/
	u32 dbg1;		/* 0x304 Debug 1*/
	u32 dbgcam;		/* 0x308 CAM Debug*/
	u32 dbgcmd;		/* 0x30c Command Debug*/
	u32 dbgstat;		/* 0x310 Status Debug*/
	u8 reserved314[0x320 - 0x314];
	u32 swctl;		/* 0x320 Software Programming Control Enable*/
	u32 swstat;		/* 0x324 Software Programming Control Status*/
	u8 reserved328[0x36c - 0x328];
	u32 poisoncfg;		/* 0x36c AXI Poison Configuration Register*/
	u32 poisonstat;		/* 0x370 AXI Poison Status Register*/
	u8 reserved374[0x3fc - 0x374];

	/* Multi Port registers */
	u32 pstat;		/* 0x3fc Port Status*/
	u32 pccfg;		/* 0x400 Port Common Configuration*/

	/* PORT 0 */
	u32 pcfgr_0;		/* 0x404 Configuration Read*/
	u32 pcfgw_0;		/* 0x408 Configuration Write*/
	u8 reserved40c[0x490 - 0x40c];
	u32 pctrl_0;		/* 0x490 Port Control Register */
	u32 pcfgqos0_0;		/* 0x494 Read QoS Configuration 0*/
	u32 pcfgqos1_0;		/* 0x498 Read QoS Configuration 1*/
	u32 pcfgwqos0_0;	/* 0x49c Write QoS Configuration 0*/
	u32 pcfgwqos1_0;	/* 0x4a0 Write QoS Configuration 1*/
	u8 reserved4a4[0x4b4 - 0x4a4];

	/* PORT 1 */
	u32 pcfgr_1;		/* 0x4b4 Configuration Read*/
	u32 pcfgw_1;		/* 0x4b8 Configuration Write*/
	u8 reserved4bc[0x540 - 0x4bc];
	u32 pctrl_1;		/* 0x540 Port 2 Control Register */
	u32 pcfgqos0_1;		/* 0x544 Read QoS Configuration 0*/
	u32 pcfgqos1_1;		/* 0x548 Read QoS Configuration 1*/
	u32 pcfgwqos0_1;	/* 0x54c Write QoS Configuration 0*/
	u32 pcfgwqos1_1;	/* 0x550 Write QoS Configuration 1*/
};

/* DDR Physical Interface Control (DDRPHYC) registers*/
struct stm32mp1_ddrphy {
	u32 ridr;		/* 0x00 R Revision Identification*/
	u32 pir;		/* 0x04 R/W PHY Initialization*/
	u32 pgcr;		/* 0x08 R/W PHY General Configuration*/
	u32 pgsr;		/* 0x0C PHY General Status*/
	u32 dllgcr;		/* 0x10 R/W DLL General Control*/
	u32 acdllcr;		/* 0x14 R/W AC DLL Control*/
	u32 ptr0;		/* 0x18 R/W PHY Timing 0*/
	u32 ptr1;		/* 0x1C R/W PHY Timing 1*/
	u32 ptr2;		/* 0x20 R/W PHY Timing 2*/
	u32 aciocr;		/* 0x24 AC I/O Configuration*/
	u32 dxccr;		/* 0x28 DATX8 Common Configuration*/
	u32 dsgcr;		/* 0x2C DDR System General Configuration*/
	u32 dcr;		/* 0x30 DRAM Configuration*/
	u32 dtpr0;		/* 0x34 DRAM Timing Parameters0*/
	u32 dtpr1;		/* 0x38 DRAM Timing Parameters1*/
	u32 dtpr2;		/* 0x3C DRAM Timing Parameters2*/
	u32 mr0;		/* 0x40 Mode 0*/
	u32 mr1;		/* 0x44 Mode 1*/
	u32 mr2;		/* 0x48 Mode 2*/
	u32 mr3;		/* 0x4C Mode 3*/
	u32 odtcr;		/* 0x50 ODT Configuration*/
	u32 dtar;		/* 0x54 data training address*/
	u32 dtdr0;		/* 0x58 */
	u32 dtdr1;		/* 0x5c */
	u8 res1[0x0c0 - 0x060];	/* 0x60 */
	u32 dcuar;		/* 0xc0 Address*/
	u32 dcudr;		/* 0xc4 DCU Data*/
	u32 dcurr;		/* 0xc8 DCU Run*/
	u32 dculr;		/* 0xcc DCU Loop*/
	u32 dcugcr;		/* 0xd0 DCU General Configuration*/
	u32 dcutpr;		/* 0xd4 DCU Timing Parameters */
	u32 dcusr0;		/* 0xd8 DCU Status 0*/
	u32 dcusr1;		/* 0xdc DCU Status 1*/
	u8 res2[0x100 - 0xe0];	/* 0xe0 */
	u32 bistrr;		/* 0x100 BIST Run*/
	u32 bistmskr0;		/* 0x104 BIST Mask 0*/
	u32 bistmskr1;		/* 0x108 BIST Mask 0*/
	u32 bistwcr;		/* 0x10c BIST Word Count*/
	u32 bistlsr;		/* 0x110 BIST LFSR Seed*/
	u32 bistar0;		/* 0x114 BIST Address 0*/
	u32 bistar1;		/* 0x118 BIST Address 1*/
	u32 bistar2;		/* 0x11c BIST Address 2*/
	u32 bistupdr;		/* 0x120 BIST User Data Pattern*/
	u32 bistgsr;		/* 0x124 BIST General Status*/
	u32 bistwer;		/* 0x128 BIST Word Error*/
	u32 bistber0;		/* 0x12c BIST Bit Error 0*/
	u32 bistber1;		/* 0x130 BIST Bit Error 1*/
	u32 bistber2;		/* 0x134 BIST Bit Error 2*/
	u32 bistwcsr;		/* 0x138 BIST Word Count Status*/
	u32 bistfwr0;		/* 0x13c BIST Fail Word 0*/
	u32 bistfwr1;		/* 0x140 BIST Fail Word 1*/
	u8 res3[0x178 - 0x144];	/* 0x144 */
	u32 gpr0;		/* 0x178 General Purpose 0 (GPR0)*/
	u32 gpr1;		/* 0x17C General Purpose 1 (GPR1)*/
	u32 zq0cr0;		/* 0x180 zq 0 control 0 */
	u32 zq0cr1;		/* 0x184 zq 0 control 1 */
	u32 zq0sr0;		/* 0x188 zq 0 status 0 */
	u32 zq0sr1;		/* 0x18C zq 0 status 1 */
	u8 res4[0x1C0 - 0x190];	/* 0x190 */
	u32 dx0gcr;		/* 0x1c0 Byte lane 0 General Configuration*/
	u32 dx0gsr0;		/* 0x1c4 Byte lane 0 General Status 0*/
	u32 dx0gsr1;		/* 0x1c8 Byte lane 0 General Status 1*/
	u32 dx0dllcr;		/* 0x1cc Byte lane 0 DLL Control*/
	u32 dx0dqtr;		/* 0x1d0 Byte lane 0 DQ Timing*/
	u32 dx0dqstr;		/* 0x1d4 Byte lane 0 DQS Timing*/
	u8 res5[0x200 - 0x1d8];	/* 0x1d8 */
	u32 dx1gcr;		/* 0x200 Byte lane 1 General Configuration*/
	u32 dx1gsr0;		/* 0x204 Byte lane 1 General Status 0*/
	u32 dx1gsr1;		/* 0x208 Byte lane 1 General Status 1*/
	u32 dx1dllcr;		/* 0x20c Byte lane 1 DLL Control*/
	u32 dx1dqtr;		/* 0x210 Byte lane 1 DQ Timing*/
	u32 dx1dqstr;		/* 0x214 Byte lane 1 QS Timing*/
	u8 res6[0x240 - 0x218];	/* 0x218 */
	u32 dx2gcr;		/* 0x240 Byte lane 2 General Configuration*/
	u32 dx2gsr0;		/* 0x244 Byte lane 2 General Status 0*/
	u32 dx2gsr1;		/* 0x248 Byte lane 2 General Status 1*/
	u32 dx2dllcr;		/* 0x24c Byte lane 2 DLL Control*/
	u32 dx2dqtr;		/* 0x250 Byte lane 2 DQ Timing*/
	u32 dx2dqstr;		/* 0x254 Byte lane 2 QS Timing*/
	u8 res7[0x280 - 0x258];	/* 0x258 */
	u32 dx3gcr;		/* 0x280 Byte lane 3 General Configuration*/
	u32 dx3gsr0;		/* 0x284 Byte lane 3 General Status 0*/
	u32 dx3gsr1;		/* 0x288 Byte lane 3 General Status 1*/
	u32 dx3dllcr;		/* 0x28c Byte lane 3 DLL Control*/
	u32 dx3dqtr;		/* 0x290 Byte lane 3 DQ Timing*/
	u32 dx3dqstr;		/* 0x294 Byte lane 3 QS Timing*/
};

#define DXN(phy, offset, byte)	((u32)(phy) + (offset) + ((u32)(byte) * 0x40))
#define DXNGCR(phy, byte)	DXN(phy, 0x1c0, byte)
#define DXNDLLCR(phy, byte)	DXN(phy, 0x1cc, byte)
#define DXNDQTR(phy, byte)	DXN(phy, 0x1d0, byte)
#define DXNDQSTR(phy, byte)	DXN(phy, 0x1d4, byte)

/* DDRCTRL REGISTERS */
#define DDRCTRL_MSTR_DDR3			BIT(0)
#define DDRCTRL_MSTR_LPDDR2			BIT(2)
#define DDRCTRL_MSTR_LPDDR3			BIT(3)
#define DDRCTRL_MSTR_DATA_BUS_WIDTH_MASK	GENMASK(13, 12)
#define DDRCTRL_MSTR_DATA_BUS_WIDTH_SHIFT	12
#define DDRCTRL_MSTR_DATA_BUS_WIDTH_FULL	(0 << 12)
#define DDRCTRL_MSTR_DATA_BUS_WIDTH_HALF	(1 << 12)
#define DDRCTRL_MSTR_DATA_BUS_WIDTH_QUARTER	(2 << 12)
#define DDRCTRL_MSTR_DLL_OFF_MODE		BIT(15)

#define DDRCTRL_STAT_OPERATING_MODE_MASK	GENMASK(2, 0)
#define DDRCTRL_STAT_OPERATING_MODE_NORMAL	1
#define DDRCTRL_STAT_OPERATING_MODE_SR		3
#define DDRCTRL_STAT_SELFREF_TYPE_MASK		GENMASK(5, 4)
#define DDRCTRL_STAT_SELFREF_TYPE_ASR		(3 << 4)
#define DDRCTRL_STAT_SELFREF_TYPE_SR		(2 << 4)

#define DDRCTRL_MRCTRL0_MR_TYPE_WRITE		0
/* only one rank supported */
#define DDRCTRL_MRCTRL0_MR_RANK_SHIFT		4
#define DDRCTRL_MRCTRL0_MR_RANK_ALL \
		(0x1 << DDRCTRL_MRCTRL0_MR_RANK_SHIFT)
#define DDRCTRL_MRCTRL0_MR_ADDR_SHIFT		12
#define DDRCTRL_MRCTRL0_MR_ADDR_MASK		GENMASK(15, 12)
#define DDRCTRL_MRCTRL0_MR_WR			BIT(31)

#define DDRCTRL_MRSTAT_MR_WR_BUSY		BIT(0)

#define DDRCTRL_PWRCTL_SELFREF_EN		BIT(0)
#define DDRCTRL_PWRCTL_POWERDOWN_EN		BIT(1)
#define DDRCTRL_PWRCTL_EN_DFI_DRAM_CLK_DISABLE	BIT(3)
#define DDRCTRL_PWRCTL_SELFREF_SW		BIT(5)

#define DDRCTRL_PWRTMG_SELFREF_TO_X32(n)	(((n) & 0xff) << 16)
#define DDRCTRL_PWRTMG_POWERDOWN_TO_X32(n)	((n) & 0x1f)

#define DDRCTRL_HWLPCTL_HW_LP_EN		BIT(0)

#define DDRCTRL_RFSHCTL3_DIS_AUTO_REFRESH	BIT(0)

#define DDRCTRL_RFSHTMG_T_RFC_NOM_X1_X32_MASK	GENMASK(27, 16)
#define DDRCTRL_RFSHTMG_T_RFC_NOM_X1_X32_SHIFT	16

#define DDRCTRL_INIT0_SKIP_DRAM_INIT_MASK	(0xC0000000)
#define DDRCTRL_INIT0_SKIP_DRAM_INIT_NORMAL	(BIT(30))

#define DDRCTRL_DFIMISC_DFI_INIT_COMPLETE_EN	BIT(0)

#define DDRCTRL_SWCTL_SW_DONE			BIT(0)

#define DDRCTRL_SWSTAT_SW_DONE_ACK		BIT(0)

#define DDRCTRL_PCTRL_N_PORT_EN			BIT(0)

/* DDRPHYC registers */
#define DDRPHYC_PIR_INIT			BIT(0)
#define DDRPHYC_PIR_DLLSRST			BIT(1)
#define DDRPHYC_PIR_DLLLOCK			BIT(2)
#define DDRPHYC_PIR_ZCAL			BIT(3)
#define DDRPHYC_PIR_ITMSRST			BIT(4)
#define DDRPHYC_PIR_DRAMRST			BIT(5)
#define DDRPHYC_PIR_DRAMINIT			BIT(6)
#define DDRPHYC_PIR_QSTRN			BIT(7)
#define DDRPHYC_PIR_RVTRN			BIT(8)
#define DDRPHYC_PIR_ICPC			BIT(16)
#define DDRPHYC_PIR_ZCALBYP			BIT(30)

#define DDRPHYC_PGSR_IDONE			BIT(0)
#define DDRPHYC_PGSR_DTERR			BIT(5)
#define DDRPHYC_PGSR_DTIERR			BIT(6)
#define DDRPHYC_PGSR_DFTERR			BIT(7)
#define DDRPHYC_PGSR_RVERR			BIT(8)
#define DDRPHYC_PGSR_RVEIRR			BIT(9)

/* PWR registers */
#define PWR_CR3					0x00C
#define PWR_CR3_DDRSRDIS			BIT(11)
#define PWR_CR3_DDRRETEN			BIT(12)

#endif
