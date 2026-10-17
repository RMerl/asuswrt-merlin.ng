// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) ASPEED Technology Inc.
 */
#include <common.h>
#include <clk.h>
#include <dm.h>
#include <errno.h>
#include <ram.h>
#include <regmap.h>
#include <reset.h>
#include <asm/io.h>
#include <asm/arch/scu_ast2600.h>
#include <asm/arch/sdram_ast2600.h>
#include <asm/global_data.h>
#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/bitfield.h>
#include <dt-bindings/clock/ast2600-clock.h>

#define DDR_PHY_TBL_CHG_ADDR            0xaeeddeea
#define DDR_PHY_TBL_END                 0xaeededed

/**
 * phyr030[18:16] - Ron PU (PHY side)
 * phyr030[14:12] - Ron PD (PHY side)
 *   b'000 : disable
 *   b'001 : 240 ohm
 *   b'010 : 120 ohm
 *   b'011 : 80 ohm
 *   b'100 : 60 ohm
 *   b'101 : 48 ohm
 *   b'110 : 40 ohm
 *   b'111 : 34 ohm (default)
 */
#define PHY_RON				((0x7 << 16) | (0x7 << 12))

/**
 * phyr030[10:8] - ODT configuration (PHY side)
 *   b'000 : ODT disabled
 *   b'001 : 240 ohm
 *   b'010 : 120 ohm
 *   b'011 : 80 ohm (default)
 *   b'100 : 60 ohm
 *   b'101 : 48 ohm
 *   b'110 : 40 ohm
 *   b'111 : 34 ohm
 */
#define PHY_ODT				(0x3 << 8)

/**
 * MR1[2:1] output driver impedance
 *   b'00 : 34 ohm (default)
 *   b'01 : 48 ohm
 */
#define DRAM_RON			(0x0 << 1)

/**
 * DRAM ODT - synchronous ODT mode
 *   RTT_WR: disable
 *   RTT_NOM = RTT_PARK
 *
 * MR1[10:8] RTT_NOM
 *   b'000 : RTT_NOM disable
 *   b'001 : 60 ohm
 *   b'010 : 120 ohm
 *   b'011 : 40 ohm
 *   b'100 : 240 ohm
 *   b'101 : 48 ohm  (default)
 *   b'110 : 80 ohm
 *   b'111 : 34 ohm
 *
 * MR5[8:6] RTT_PARK
 *   b'000 : RTT_PARK disable
 *   b'001 : 60 ohm
 *   b'010 : 120 ohm
 *   b'011 : 40 ohm
 *   b'100 : 240 ohm
 *   b'101 : 48 ohm  (default)
 *   b'110 : 80 ohm
 *   b'111 : 34 ohm
 *
 * MR2[11:9] RTT_WR
 *   b'000 : Dynamic ODT off  (default)
 *   b'001 : 120 ohm
 *   b'010 : 240 ohm
 *   b'011 : Hi-Z
 *   b'100 : 80 ohm
 */
#define RTT_WR				(0x0 << 9)
#define RTT_NOM				(0x5 << 8)
#define RTT_PARK			(0x5 << 6)

/**
 * MR6[6] VrefDQ training range
 *   b'0 : range 1
 *   b'1 : range 2 (default)
 */
#define VREFDQ_RANGE_2			BIT(6)

/**
 * Latency setting:
 * AL = PL = 0 (hardware fixed setting)
 * -> WL = AL + CWL + PL = CWL
 * -> RL = AL + CL + PL = CL
 */
#define CFG_WL			9
#define CFG_RL			12
#define T_RDDATA_EN			((CFG_RL - 2) << 8)
#define T_PHY_WRLAT			(CFG_WL - 2)

/* MR0 */
#define MR0_CL_12			(BIT(4) | BIT(2))
#define MR0_WR12_RTP6			BIT(9)
#define MR0_DLL_RESET			BIT(8)
#define MR0_VAL				(MR0_CL_12 | MR0_WR12_RTP6 | MR0_DLL_RESET)

/* MR1 */
#define MR1_VAL				(0x0001 | RTT_NOM | DRAM_RON)

/* MR2 */
#define MR2_CWL_9			0
#define MR2_VAL				(0x0000 | RTT_WR | MR2_CWL_9)

/* MR3 ~ MR6 */
#define MR3_VAL				0x0000
#define MR4_VAL				0x0000
#define MR5_VAL				(0x0400 | RTT_PARK)
#define MR6_VAL				0x0400

/**
 * The offset value applied to the DDR PHY write data eye training result
 * to fine-tune the write DQ/DQS alignment
 */
#define WR_DATA_EYE_OFFSET		(0x10 << 8)

#if defined(CONFIG_ASPEED_DDR4_800)
u32 ast2600_sdramphy_config[165] = {
	0x1e6e0100,	// start address
	0x00000000,	// phyr000
	0x0c002062,	// phyr004
	0x1a7a0063,	// phyr008
	0x5a7a0063,	// phyr00c
	0x1a7a0063,	// phyr010
	0x1a7a0063,	// phyr014
	0x20000000,	// phyr018
	0x20000000,	// phyr01c
	0x20000000,	// phyr020
	0x20000000,	// phyr024
	0x00000008,	// phyr028
	0x00000000,	// phyr02c
	(PHY_RON | PHY_ODT),	/* phyr030 */
	0x00000000,	// phyr034
	0x00000000,	// phyr038
	0x20000000,	// phyr03c
	0x50506000,	// phyr040
	0x50505050,	// phyr044
	0x00002f07,	// phyr048
	0x00003080,	// phyr04c
	0x04000000,	// phyr050
	((MR3_VAL << 16) | MR2_VAL),	/* phyr054 */
	((MR0_VAL << 16) | MR1_VAL),	/* phyr058 */
	((MR5_VAL << 16) | MR4_VAL),	/* phyr05c */
	((0x0800 << 16) | MR6_VAL | VREFDQ_RANGE_2 | 0xe), /* phyr060 */
	0x00000000,	// phyr064
	0x00180008,	// phyr068
	0x00e00400,	// phyr06c
	0x00140206,	// phyr070
	0x1d4c0000,	// phyr074
	(0x493e0100 | T_PHY_WRLAT),	/* phyr078 */
	0x08060404,	// phyr07c
	(0x90000000 | T_RDDATA_EN),	/* phyr080 */
	0x06420618,	// phyr084
	0x00001002,	// phyr088
	0x05701016,	// phyr08c
	0x10000000,	// phyr090
	0xaeeddeea,	// change address
	0x1e6e019c,	// new address
	0x20202020,	// phyr09c
	0x20202020,	// phyr0a0
	0x00002020,	// phyr0a4
	0x00002020,	// phyr0a8
	0x00000001,	// phyr0ac
	0xaeeddeea,	// change address
	0x1e6e01cc,	// new address
	0x01010101,	// phyr0cc
	0x01010101,	// phyr0d0
	0x80808080,	// phyr0d4
	0x80808080,	// phyr0d8
	0xaeeddeea,	// change address
	0x1e6e0288,	// new address
	0x80808080,	// phyr188
	0x80808080,	// phyr18c
	0x80808080,	// phyr190
	0x80808080,	// phyr194
	0xaeeddeea,	// change address
	0x1e6e02f8,	// new address
	0x90909090,	// phyr1f8
	0x88888888,	// phyr1fc
	0xaeeddeea,	// change address
	0x1e6e0300,	// new address
	0x00000000,	// phyr200
	0xaeeddeea,	// change address
	0x1e6e0194,	// new address
	0x80118260,	// phyr094
	0xaeeddeea,	// change address
	0x1e6e019c,	// new address
	0x20202020,	// phyr09c
	0x20202020,	// phyr0a0
	0x00002020,	// phyr0a4
	0x00000000,	/* phyr0a8 */
	0x00000001,	// phyr0ac
	0xaeeddeea,	// change address
	0x1e6e0318,	// new address
	0x09222719,	// phyr218
	0x00aa4403,	// phyr21c
	0xaeeddeea,	// change address
	0x1e6e0198,	// new address
	0x08060000,	// phyr098
	0xaeeddeea,	// change address
	0x1e6e01b0,	// new address
	0x00000000,	// phyr0b0
	0x00000000,	// phyr0b4
	0x00000000,	// phyr0b8
	0x00000000,	// phyr0bc
	0x00000000,	// phyr0c0
	0x00000000,	// phyr0c4
	0x000aff2c,	// phyr0c8
	0xaeeddeea,	// change address
	0x1e6e01dc,	// new address
	0x00080000,	// phyr0dc
	0x00000000,	// phyr0e0
	0xaa55aa55,	// phyr0e4
	0x55aa55aa,	// phyr0e8
	0xaaaa5555,	// phyr0ec
	0x5555aaaa,	// phyr0f0
	0xaa55aa55,	// phyr0f4
	0x55aa55aa,	// phyr0f8
	0xaaaa5555,	// phyr0fc
	0x5555aaaa,	// phyr100
	0xaa55aa55,	// phyr104
	0x55aa55aa,	// phyr108
	0xaaaa5555,	// phyr10c
	0x5555aaaa,	// phyr110
	0xaa55aa55,	// phyr114
	0x55aa55aa,	// phyr118
	0xaaaa5555,	// phyr11c
	0x5555aaaa,	// phyr120
	0x20202020,	// phyr124
	0x20202020,	// phyr128
	0x20202020,	// phyr12c
	0x20202020,	// phyr130
	0x20202020,	// phyr134
	0x20202020,	// phyr138
	0x20202020,	// phyr13c
	0x20202020,	// phyr140
	0x20202020,	// phyr144
	0x20202020,	// phyr148
	0x20202020,	// phyr14c
	0x20202020,	// phyr150
	0x20202020,	// phyr154
	0x20202020,	// phyr158
	0x20202020,	// phyr15c
	0x20202020,	// phyr160
	0x20202020,	// phyr164
	0x20202020,	// phyr168
	0x20202020,	// phyr16c
	0x20202020,	// phyr170
	0xaeeddeea,	// change address
	0x1e6e0298,	// new address
	0x20200000,	/* phyr198 */
	0x20202020,	// phyr19c
	0x20202020,	// phyr1a0
	0x20202020,	// phyr1a4
	0x20202020,	// phyr1a8
	0x20202020,	// phyr1ac
	0x20202020,	// phyr1b0
	0x20202020,	// phyr1b4
	0x20202020,	// phyr1b8
	0x20202020,	// phyr1bc
	0x20202020,	// phyr1c0
	0x20202020,	// phyr1c4
	0x20202020,	// phyr1c8
	0x20202020,	// phyr1cc
	0x20202020,	// phyr1d0
	0x20202020,	// phyr1d4
	0x20202020,	// phyr1d8
	0x20202020,	// phyr1dc
	0x20202020,	// phyr1e0
	0x20202020,	// phyr1e4
	0x00002020,	// phyr1e8
	0xaeeddeea,	// change address
	0x1e6e0304,	// new address
	(0x00000001 | WR_DATA_EYE_OFFSET), /* phyr204 */
	0xaeeddeea,	// change address
	0x1e6e027c,	// new address
	0x4e400000,	// phyr17c
	0x59595959,	// phyr180
	0x40404040,	// phyr184
	0xaeeddeea,	// change address
	0x1e6e02f4,	// new address
	0x00000059,	// phyr1f4
	0xaeededed,	// end
};
#else
u32 ast2600_sdramphy_config[165] = {
	0x1e6e0100,	// start address
	0x00000000,	// phyr000
	0x0c002062,	// phyr004
	0x1a7a0063,	// phyr008
	0x5a7a0063,	// phyr00c
	0x1a7a0063,	// phyr010
	0x1a7a0063,	// phyr014
	0x20000000,	// phyr018
	0x20000000,	// phyr01c
	0x20000000,	// phyr020
	0x20000000,	// phyr024
	0x00000008,	// phyr028
	0x00000000,	// phyr02c
	(PHY_RON | PHY_ODT),	/* phyr030 */
	0x00000000,	// phyr034
	0x00000000,	// phyr038
	0x20000000,	// phyr03c
	0x50506000,	// phyr040
	0x50505050,	// phyr044
	0x00002f07,	// phyr048
	0x00003080,	// phyr04c
	0x04000000,	// phyr050
	((MR3_VAL << 16) | MR2_VAL),	/* phyr054 */
	((MR0_VAL << 16) | MR1_VAL),	/* phyr058 */
	((MR5_VAL << 16) | MR4_VAL),	/* phyr05c */
	((0x0800 << 16) | MR6_VAL | VREFDQ_RANGE_2 | 0xe), /* phyr060 */
	0x00000000,	// phyr064
	0x00180008,	// phyr068
	0x00e00400,	// phyr06c
	0x00140206,	// phyr070
	0x1d4c0000,	// phyr074
	(0x493e0100 | T_PHY_WRLAT),	/* phyr078 */
	0x08060404,	// phyr07c
	(0x90000000 | T_RDDATA_EN),	/* phyr080 */
	0x06420c30,	// phyr084
	0x00001002,	// phyr088
	0x05701016,	// phyr08c
	0x10000000,	// phyr090
	0xaeeddeea,	// change address
	0x1e6e019c,	// new address
	0x20202020,	// phyr09c
	0x20202020,	// phyr0a0
	0x00002020,	// phyr0a4
	0x00002020,	// phyr0a8
	0x00000001,	// phyr0ac
	0xaeeddeea,	// change address
	0x1e6e01cc,	// new address
	0x01010101,	// phyr0cc
	0x01010101,	// phyr0d0
	0x80808080,	// phyr0d4
	0x80808080,	// phyr0d8
	0xaeeddeea,	// change address
	0x1e6e0288,	// new address
	0x80808080,	// phyr188
	0x80808080,	// phyr18c
	0x80808080,	// phyr190
	0x80808080,	// phyr194
	0xaeeddeea,	// change address
	0x1e6e02f8,	// new address
	0x90909090,	// phyr1f8
	0x88888888,	// phyr1fc
	0xaeeddeea,	// change address
	0x1e6e0300,	// new address
	0x00000000,	// phyr200
	0xaeeddeea,	// change address
	0x1e6e0194,	// new address
	0x801112e0,	// phyr094
	0xaeeddeea,	// change address
	0x1e6e019c,	// new address
	0x20202020,	// phyr09c
	0x20202020,	// phyr0a0
	0x00002020,	// phyr0a4
	0x00000000,	/* phyr0a8 */
	0x00000001,	// phyr0ac
	0xaeeddeea,	// change address
	0x1e6e0318,	// new address
	0x09222719,	// phyr218
	0x00aa4403,	// phyr21c
	0xaeeddeea,	// change address
	0x1e6e0198,	// new address
	0x08060000,	// phyr098
	0xaeeddeea,	// change address
	0x1e6e01b0,	// new address
	0x00000000,	// phyr0b0
	0x00000000,	// phyr0b4
	0x00000000,	// phyr0b8
	0x00000000,	// phyr0bc
	0x00000000,	// phyr0c0 - ori
	0x00000000,	// phyr0c4
	0x000aff2c,	// phyr0c8
	0xaeeddeea,	// change address
	0x1e6e01dc,	// new address
	0x00080000,	// phyr0dc
	0x00000000,	// phyr0e0
	0xaa55aa55,	// phyr0e4
	0x55aa55aa,	// phyr0e8
	0xaaaa5555,	// phyr0ec
	0x5555aaaa,	// phyr0f0
	0xaa55aa55,	// phyr0f4
	0x55aa55aa,	// phyr0f8
	0xaaaa5555,	// phyr0fc
	0x5555aaaa,	// phyr100
	0xaa55aa55,	// phyr104
	0x55aa55aa,	// phyr108
	0xaaaa5555,	// phyr10c
	0x5555aaaa,	// phyr110
	0xaa55aa55,	// phyr114
	0x55aa55aa,	// phyr118
	0xaaaa5555,	// phyr11c
	0x5555aaaa,	// phyr120
	0x20202020,	// phyr124
	0x20202020,	// phyr128
	0x20202020,	// phyr12c
	0x20202020,	// phyr130
	0x20202020,	// phyr134
	0x20202020,	// phyr138
	0x20202020,	// phyr13c
	0x20202020,	// phyr140
	0x20202020,	// phyr144
	0x20202020,	// phyr148
	0x20202020,	// phyr14c
	0x20202020,	// phyr150
	0x20202020,	// phyr154
	0x20202020,	// phyr158
	0x20202020,	// phyr15c
	0x20202020,	// phyr160
	0x20202020,	// phyr164
	0x20202020,	// phyr168
	0x20202020,	// phyr16c
	0x20202020,	// phyr170
	0xaeeddeea,	// change address
	0x1e6e0298,	// new address
	0x20200000,	/* phyr198 */
	0x20202020,	// phyr19c
	0x20202020,	// phyr1a0
	0x20202020,	// phyr1a4
	0x20202020,	// phyr1a8
	0x20202020,	// phyr1ac
	0x20202020,	// phyr1b0
	0x20202020,	// phyr1b4
	0x20202020,	// phyr1b8
	0x20202020,	// phyr1bc
	0x20202020,	// phyr1c0
	0x20202020,	// phyr1c4
	0x20202020,	// phyr1c8
	0x20202020,	// phyr1cc
	0x20202020,	// phyr1d0
	0x20202020,	// phyr1d4
	0x20202020,	// phyr1d8
	0x20202020,	// phyr1dc
	0x20202020,	// phyr1e0
	0x20202020,	// phyr1e4
	0x00002020,	// phyr1e8
	0xaeeddeea,	// change address
	0x1e6e0304,	// new address
	(0x00000001 | WR_DATA_EYE_OFFSET), /* phyr204 */
	0xaeeddeea,	// change address
	0x1e6e027c,	// new address
	0x4e400000,	// phyr17c
	0x59595959,	// phyr180
	0x40404040,	// phyr184
	0xaeeddeea,	// change address
	0x1e6e02f4,	// new address
	0x00000059,	// phyr1f4
	0xaeededed,	// end
};
#endif

/* MPLL configuration */
#define SCU_MPLL_FREQ_400M	0x0008405F
#define SCU_MPLL_EXT_400M	0x0000002F
#define SCU_MPLL_FREQ_333M	0x00488299
#define SCU_MPLL_EXT_333M	0x0000014C
#define SCU_MPLL_FREQ_200M	0x0078007F
#define SCU_MPLL_EXT_200M	0x0000003F
#define SCU_MPLL_FREQ_100M	0x0078003F
#define SCU_MPLL_EXT_100M	0x0000001F

#if defined(CONFIG_ASPEED_DDR4_1600)
#define SCU_MPLL_FREQ_CFG	SCU_MPLL_FREQ_400M
#define SCU_MPLL_EXT_CFG	SCU_MPLL_EXT_400M
#elif defined(CONFIG_ASPEED_DDR4_1333)
#define SCU_MPLL_FREQ_CFG	SCU_MPLL_FREQ_333M
#define SCU_MPLL_EXT_CFG	SCU_MPLL_EXT_333M
#elif defined(CONFIG_ASPEED_DDR4_800)
#define SCU_MPLL_FREQ_CFG	SCU_MPLL_FREQ_200M
#define SCU_MPLL_EXT_CFG	SCU_MPLL_EXT_200M
#elif defined(CONFIG_ASPEED_DDR4_400)
#define SCU_MPLL_FREQ_CFG	SCU_MPLL_FREQ_100M
#define SCU_MPLL_EXT_CFG	SCU_MPLL_EXT_100M
#else
#error "undefined DDR4 target rate\n"
#endif

/*
 * AC timing and SDRAM mode register setting
 * for real chip are derived from the model GDDR4-1600
 */
#define DDR4_MR01_MODE	((MR1_VAL << 16) | MR0_VAL)
#define DDR4_MR23_MODE	((MR3_VAL << 16) | MR2_VAL)
#define DDR4_MR45_MODE	((MR5_VAL << 16) | MR4_VAL)
#define DDR4_MR6_MODE	MR6_VAL
#define DDR4_TRFC_1600	0x467299f1
#define DDR4_TRFC_1333	0x3a5f80c9
#define DDR4_TRFC_800	0x23394c78
#define DDR4_TRFC_400	0x111c263c

#if defined(CONFIG_ASPEED_DDR4_1600)
#define DDR4_TRFC		DDR4_TRFC_1600
#define DDR4_PHY_TRAIN_TRFC	0xc30
#elif defined(CONFIG_ASPEED_DDR4_1333)
#define DDR4_TRFC		DDR4_TRFC_1333
#define DDR4_PHY_TRAIN_TRFC	0xa25
#elif defined(CONFIG_ASPEED_DDR4_800)
#define DDR4_TRFC		DDR4_TRFC_800
#define DDR4_PHY_TRAIN_TRFC	0x618
#elif defined(CONFIG_ASPEED_DDR4_400)
#define DDR4_TRFC		DDR4_TRFC_400
#define DDR4_PHY_TRAIN_TRFC	0x30c
#else
#error "undefined tRFC setting"
#endif

/* supported SDRAM size */
#define SDRAM_SIZE_1KB		(1024U)
#define SDRAM_SIZE_1MB		(SDRAM_SIZE_1KB * SDRAM_SIZE_1KB)
#define SDRAM_MIN_SIZE		(256 * SDRAM_SIZE_1MB)
#define SDRAM_MAX_SIZE		(2048 * SDRAM_SIZE_1MB)

DECLARE_GLOBAL_DATA_PTR;

static const u32 ddr4_ac_timing[4] = {
	0x040e0307, 0x0f4711f1, 0x0e060304, 0x00001240 };
static const u32 ddr_max_grant_params[4] = {
	0x44444444, 0x44444444, 0x44444444, 0x44444444 };

struct dram_info {
	struct ram_info info;
	struct clk ddr_clk;
	struct ast2600_sdrammc_regs *regs;
	struct ast2600_scu *scu;
	struct ast2600_ddr_phy *phy;
	void __iomem *phy_setting;
	void __iomem *phy_status;
	ulong clock_rate;
};

static void ast2600_sdramphy_kick_training(struct dram_info *info)
{
	u32 data;
	struct ast2600_sdrammc_regs *regs = info->regs;

	writel(SDRAM_PHYCTRL0_NRST, &regs->phy_ctrl[0]);
	udelay(5);
	writel(SDRAM_PHYCTRL0_NRST | SDRAM_PHYCTRL0_INIT, &regs->phy_ctrl[0]);
	udelay(1000);

	while (1) {
		data = readl(&regs->phy_ctrl[0]) & SDRAM_PHYCTRL0_INIT;
		if (data == 0)
			break;
	}
}

/**
 * @brief	load DDR-PHY configurations table to the PHY registers
 * @param[in]	p_tbl - pointer to the configuration table
 * @param[in]	info - pointer to the DRAM info struct
 *
 * There are two sets of MRS (Mode Registers) configuration in ast2600 memory
 * system: one is in the SDRAM MC (memory controller) which is used in run
 * time, and the other is in the DDR-PHY IP which is used during DDR-PHY
 * training.
 */
static void ast2600_sdramphy_init(u32 *p_tbl, struct dram_info *info)
{
	u32 reg_base = (u32)info->phy_setting;
	u32 addr = p_tbl[0];
	u32 data;
	int i = 1;

	writel(0, &info->regs->phy_ctrl[0]);
	udelay(10);

	while (1) {
		if (addr < reg_base) {
			debug("invalid DDR-PHY addr: 0x%08x\n", addr);
			break;
		}
		data = p_tbl[i++];

		if (data == DDR_PHY_TBL_END) {
			break;
		} else if (data == DDR_PHY_TBL_CHG_ADDR) {
			addr = p_tbl[i++];
		} else {
			writel(data, addr);
			addr += 4;
		}
	}

	data = readl(info->phy_setting + 0x84) & ~GENMASK(16, 0);
	data |= DDR4_PHY_TRAIN_TRFC;
	writel(data, info->phy_setting + 0x84);
}

static int ast2600_sdramphy_check_status(struct dram_info *info)
{
	u32 value, tmp;
	u32 reg_base = (u32)info->phy_status;
	int need_retrain = 0;

	debug("\nSDRAM PHY training report:\n");

	/* training status */
	value = readl(reg_base + 0x00);
	debug("rO_DDRPHY_reg offset 0x00 = 0x%08x\n", value);

	if (value & BIT(3))
		debug("\tinitial PVT calibration fail\n");

	if (value & BIT(5))
		debug("\truntime calibration fail\n");

	/* PU & PD */
	value = readl(reg_base + 0x30);
	debug("rO_DDRPHY_reg offset 0x30 = 0x%08x\n", value);
	debug("  PU = 0x%02x\n", value & 0xff);
	debug("  PD = 0x%02x\n", (value >> 16) & 0xff);

	/* read eye window */
	value = readl(reg_base + 0x68);
	if (0 == (value & GENMASK(7, 0)))
		need_retrain = 1;

	debug("rO_DDRPHY_reg offset 0x68 = 0x%08x\n", value);
	debug("  rising edge of read data eye training pass window\n");
	tmp = (((value & GENMASK(7, 0)) >> 0) * 100) / 255;
	debug("    B0:%d%%\n", tmp);
	tmp = (((value & GENMASK(15, 8)) >> 8) * 100) / 255;
	debug("    B1:%d%%\n", tmp);

	value = readl(reg_base + 0xC8);
	debug("rO_DDRPHY_reg offset 0xC8 = 0x%08x\n", value);
	debug("  falling edge of read data eye training pass window\n");
	tmp = (((value & GENMASK(7, 0)) >> 0) * 100) / 255;
	debug("    B0:%d%%\n", tmp);
	tmp = (((value & GENMASK(15, 8)) >> 8) * 100) / 255;
	debug("    B1:%d%%\n", tmp);

	/* write eye window */
	value = readl(reg_base + 0x7c);
	if (0 == (value & GENMASK(7, 0)))
		need_retrain = 1;

	debug("rO_DDRPHY_reg offset 0x7C = 0x%08x\n", value);
	debug("  rising edge of write data eye training pass window\n");
	tmp = (((value & GENMASK(7, 0)) >> 0) * 100) / 255;
	debug("    B0:%d%%\n", tmp);
	tmp = (((value & GENMASK(15, 8)) >> 8) * 100) / 255;
	debug("    B1:%d%%\n", tmp);

	/* read Vref training result */
	value = readl(reg_base + 0x88);
	debug("rO_DDRPHY_reg offset 0x88 = 0x%08x\n", value);
	debug("  read Vref training result\n");
	tmp = (((value & GENMASK(7, 0)) >> 0) * 100) / 127;
	debug("    B0:%d%%\n", tmp);
	tmp = (((value & GENMASK(15, 8)) >> 8) * 100) / 127;
	debug("    B1:%d%%\n", tmp);

	/* write Vref training result */
	value = readl(reg_base + 0x90);
	debug("rO_DDRPHY_reg offset 0x90 = 0x%08x\n", value);

	/* gate train */
	value = readl(reg_base + 0x50);
	if ((0 == (value & GENMASK(15, 0))) ||
	    (0 == (value & GENMASK(31, 16)))) {
		need_retrain = 1;
	}

	debug("rO_DDRPHY_reg offset 0x50 = 0x%08x\n", value);

	return need_retrain;
}

#ifndef CONFIG_ASPEED_BYPASS_SELFTEST
#define MC_TEST_PATTERN_N 8
static u32 as2600_sdrammc_test_pattern[MC_TEST_PATTERN_N] = {
	0xcc33cc33, 0xff00ff00, 0xaa55aa55, 0x88778877,
	0x92cc4d6e, 0x543d3cde, 0xf1e843c7, 0x7c61d253 };

#define TIMEOUT_DRAM	5000000
int ast2600_sdrammc_dg_test(struct dram_info *info, unsigned int datagen, u32 mode)
{
	unsigned int data;
	unsigned int timeout = 0;
	struct ast2600_sdrammc_regs *regs = info->regs;

	writel(0, &regs->ecc_test_ctrl);

	if (mode == 0)
		writel(0x00000085 | (datagen << 3), &regs->ecc_test_ctrl);
	else
		writel(0x000000C1 | (datagen << 3), &regs->ecc_test_ctrl);

	do {
		data = readl(&regs->ecc_test_ctrl) & GENMASK(13, 12);

		if (data & BIT(13))
			return 0;

		if (++timeout > TIMEOUT_DRAM) {
			debug("Timeout!!\n");
			writel(0, &regs->ecc_test_ctrl);
			return -1;
		}
	} while (!data);

	writel(0, &regs->ecc_test_ctrl);

	return 0;
}

int ast2600_sdrammc_cbr_test(struct dram_info *info)
{
	u32 i;
	struct ast2600_sdrammc_regs *regs = info->regs;

	clrsetbits_le32(&regs->test_addr, GENMASK(30, 4), 0x7ffff0);

	/* single */
	for (i = 0; i < 8; i++)
		if (ast2600_sdrammc_dg_test(info, i, 0))
			return -1;

	/* burst */
	for (i = 0; i < 8; i++)
		if (ast2600_sdrammc_dg_test(info, i, i))
			return -1;

	return 0;
}

static int ast2600_sdrammc_test(struct dram_info *info)
{
	struct ast2600_sdrammc_regs *regs = info->regs;

	u32 pass_cnt = 0;
	u32 fail_cnt = 0;
	u32 target_cnt = 2;
	u32 test_cnt = 0;
	u32 pattern;
	u32 i = 0;
	bool finish = false;

	debug("sdram mc test:\n");
	while (!finish) {
		pattern = as2600_sdrammc_test_pattern[i++];
		i = i % MC_TEST_PATTERN_N;
		debug("  pattern = %08X : ", pattern);
		writel(pattern, &regs->test_init_val);

		if (ast2600_sdrammc_cbr_test(info)) {
			debug("fail\n");
			fail_cnt++;
		} else {
			debug("pass\n");
			pass_cnt++;
		}

		if (++test_cnt == target_cnt)
			finish = true;
	}
	debug("statistics: pass/fail/total:%d/%d/%d\n", pass_cnt, fail_cnt,
	      target_cnt);

	return fail_cnt;
}
#endif

/*
 * scu500[14:13]
 *	2b'00: VGA memory size = 16MB
 *	2b'01: VGA memory size = 16MB
 *	2b'10: VGA memory size = 32MB
 *	2b'11: VGA memory size = 64MB
 *
 * mcr04[3:2]
 *	2b'00: VGA memory size = 8MB
 *	2b'01: VGA memory size = 16MB
 *	2b'10: VGA memory size = 32MB
 *	2b'11: VGA memory size = 64MB
 */
static size_t ast2600_sdrammc_get_vga_mem_size(struct dram_info *info)
{
	u32 vga_hwconf;
	size_t vga_mem_size_base = 8 * 1024 * 1024;

	vga_hwconf =
		(readl(&info->scu->hwstrap1) & SCU_HWSTRAP1_VGA_MEM_MASK) >>
		 SCU_HWSTRAP1_VGA_MEM_SHIFT;

	if (vga_hwconf == 0) {
		vga_hwconf = 1;
		writel(vga_hwconf << SCU_HWSTRAP1_VGA_MEM_SHIFT,
		       &info->scu->hwstrap1);
	}

	clrsetbits_le32(&info->regs->config, SDRAM_CONF_VGA_SIZE_MASK,
			((vga_hwconf << SDRAM_CONF_VGA_SIZE_SHIFT) &
			 SDRAM_CONF_VGA_SIZE_MASK));

	/* no need to reserve VGA memory if efuse[VGA disable] is set */
	if (readl(&info->scu->efuse) & SCU_EFUSE_DIS_VGA)
		return 0;

	return vga_mem_size_base << vga_hwconf;
}

/*
 * Find out RAM size and save it in dram_info
 *
 * The procedure is taken from Aspeed SDK
 */
static void ast2600_sdrammc_calc_size(struct dram_info *info)
{
	/* The controller supports 256/512/1024/2048 MB ram */
	size_t ram_size = SDRAM_MIN_SIZE;
	const int write_test_offset = 0x100000;
	u32 test_pattern = 0xdeadbeef;
	u32 cap_param = SDRAM_CONF_CAP_2048M;
	u32 refresh_timing_param = DDR4_TRFC;
	const u32 write_addr_base = CFG_SYS_SDRAM_BASE + write_test_offset;

	for (ram_size = SDRAM_MAX_SIZE; ram_size > SDRAM_MIN_SIZE;
	     ram_size >>= 1) {
		writel(test_pattern, write_addr_base + (ram_size >> 1));
		test_pattern = (test_pattern >> 4) | (test_pattern << 28);
	}

	/* One last write to overwrite all wrapped values */
	writel(test_pattern, write_addr_base);

	/* Reset the pattern and see which value was really written */
	test_pattern = 0xdeadbeef;
	for (ram_size = SDRAM_MAX_SIZE; ram_size > SDRAM_MIN_SIZE;
	     ram_size >>= 1) {
		if (readl(write_addr_base + (ram_size >> 1)) == test_pattern)
			break;

		--cap_param;
		refresh_timing_param >>= 8;
		test_pattern = (test_pattern >> 4) | (test_pattern << 28);
	}

	clrsetbits_le32(&info->regs->ac_timing[1],
			(SDRAM_AC_TRFC_MASK << SDRAM_AC_TRFC_SHIFT),
			((refresh_timing_param & SDRAM_AC_TRFC_MASK)
			 << SDRAM_AC_TRFC_SHIFT));

	info->info.base = CFG_SYS_SDRAM_BASE;
	info->info.size = ram_size - ast2600_sdrammc_get_vga_mem_size(info);

	clrsetbits_le32(&info->regs->config, SDRAM_CONF_CAP_MASK,
			((cap_param << SDRAM_CONF_CAP_SHIFT) & SDRAM_CONF_CAP_MASK));
}

static int ast2600_sdrammc_init_ddr4(struct dram_info *info)
{
	const u32 power_ctrl = MCR34_CKE_EN | MCR34_AUTOPWRDN_EN |
		MCR34_MREQ_BYPASS_DIS | MCR34_RESETN_DIS |
		MCR34_ODT_EN | MCR34_ODT_AUTO_ON |
		(0x1 << MCR34_ODT_EXT_SHIFT);

	/* init SDRAM-PHY only on real chip */
	ast2600_sdramphy_init(ast2600_sdramphy_config, info);
	writel((MCR34_CKE_EN | MCR34_MREQI_DIS | MCR34_RESETN_DIS),
	       &info->regs->power_ctrl);
	udelay(5);
	ast2600_sdramphy_kick_training(info);
	udelay(500);
	writel(SDRAM_RESET_DLL_ZQCL_EN, &info->regs->refresh_timing);

	writel(MCR30_SET_MR(3), &info->regs->mode_setting_control);
	writel(MCR30_SET_MR(6), &info->regs->mode_setting_control);
	writel(MCR30_SET_MR(5), &info->regs->mode_setting_control);
	writel(MCR30_SET_MR(4), &info->regs->mode_setting_control);
	writel(MCR30_SET_MR(2), &info->regs->mode_setting_control);
	writel(MCR30_SET_MR(1), &info->regs->mode_setting_control);
	writel(MCR30_SET_MR(0) | MCR30_RESET_DLL_DELAY_EN,
	       &info->regs->mode_setting_control);

	writel(SDRAM_REFRESH_EN | SDRAM_RESET_DLL_ZQCL_EN |
	       (0x5f << SDRAM_REFRESH_PERIOD_SHIFT),
	       &info->regs->refresh_timing);

	/* wait self-refresh idle */
	while (readl(&info->regs->power_ctrl) &
	       MCR34_SELF_REFRESH_STATUS_MASK)
		;

	writel(SDRAM_REFRESH_EN | SDRAM_LOW_PRI_REFRESH_EN |
	       SDRAM_REFRESH_ZQCS_EN |
	       (0x5f << SDRAM_REFRESH_PERIOD_SHIFT) |
	       (0x42aa << SDRAM_REFRESH_PERIOD_ZQCS_SHIFT),
	       &info->regs->refresh_timing);

	writel(power_ctrl, &info->regs->power_ctrl);
	udelay(500);

	return 0;
}

static void ast2600_sdrammc_unlock(struct dram_info *info)
{
	writel(SDRAM_UNLOCK_KEY, &info->regs->protection_key);
	while (!readl(&info->regs->protection_key))
		;
}

static void ast2600_sdrammc_lock(struct dram_info *info)
{
	writel(~SDRAM_UNLOCK_KEY, &info->regs->protection_key);
	while (readl(&info->regs->protection_key))
		;
}

static void ast2600_sdrammc_common_init(struct ast2600_sdrammc_regs *regs)
{
	int i;
	u32 reg;

	writel(MCR34_MREQI_DIS | MCR34_RESETN_DIS, &regs->power_ctrl);
	writel(SDRAM_VIDEO_UNLOCK_KEY, &regs->gm_protection_key);
	writel(0x10 << MCR38_RW_MAX_GRANT_CNT_RQ_SHIFT,
	       &regs->arbitration_ctrl);
	writel(0xFFBBFFF4, &regs->req_limit_mask);

	for (i = 0; i < ARRAY_SIZE(ddr_max_grant_params); ++i)
		writel(ddr_max_grant_params[i], &regs->max_grant_len[i]);

	writel(MCR50_RESET_ALL_INTR, &regs->intr_ctrl);

	writel(0x07FFFFFF, &regs->ecc_range_ctrl);

	writel(0, &regs->ecc_test_ctrl);
	writel(0x80000001, &regs->test_addr);
	writel(0, &regs->test_fail_dq_bit);
	writel(0, &regs->test_init_val);

	writel(0xFFFFFFFF, &regs->req_input_ctrl);
	writel(0, &regs->req_high_pri_ctrl);

	udelay(600);

#ifdef CONFIG_ASPEED_DDR4_DUALX8
	writel(0x37, &regs->config);
#else
	writel(0x17, &regs->config);
#endif

	/* load controller setting */
	for (i = 0; i < ARRAY_SIZE(ddr4_ac_timing); ++i)
		writel(ddr4_ac_timing[i], &regs->ac_timing[i]);

	/* update CL and WL */
	reg = readl(&regs->ac_timing[1]);
	reg &= ~(SDRAM_WL_SETTING | SDRAM_CL_SETTING);
	reg |= FIELD_PREP(SDRAM_WL_SETTING, CFG_WL - 5) |
	       FIELD_PREP(SDRAM_CL_SETTING, CFG_RL - 5);
	writel(reg, &regs->ac_timing[1]);

	writel(DDR4_MR01_MODE, &regs->mr01_mode_setting);
	writel(DDR4_MR23_MODE, &regs->mr23_mode_setting);
	writel(DDR4_MR45_MODE, &regs->mr45_mode_setting);
	writel(DDR4_MR6_MODE, &regs->mr6_mode_setting);
}

/*
 * Update size info according to the ECC HW setting
 *
 * Assume SDRAM has been initialized by SPL or the host.  To get the RAM size, we
 * don't need to calculate the ECC size again but read from MCR04 and derive the
 * size from its value.
 */
static void ast2600_sdrammc_update_size(struct dram_info *info)
{
	struct ast2600_sdrammc_regs *regs = info->regs;
	u32 conf = readl(&regs->config);
	u32 cap_param;
	size_t ram_size = SDRAM_MAX_SIZE;
	size_t hw_size;

	cap_param = (conf & SDRAM_CONF_CAP_MASK) >> SDRAM_CONF_CAP_SHIFT;
	switch (cap_param) {
	case SDRAM_CONF_CAP_2048M:
		ram_size = 2048 * SDRAM_SIZE_1MB;
		break;
	case SDRAM_CONF_CAP_1024M:
		ram_size = 1024 * SDRAM_SIZE_1MB;
		break;
	case SDRAM_CONF_CAP_512M:
		ram_size = 512 * SDRAM_SIZE_1MB;
		break;
	case SDRAM_CONF_CAP_256M:
		ram_size = 256 * SDRAM_SIZE_1MB;
		break;
	}

	info->info.base = CFG_SYS_SDRAM_BASE;
	info->info.size = ram_size - ast2600_sdrammc_get_vga_mem_size(info);

	if (0 == (conf & SDRAM_CONF_ECC_SETUP))
		return;

	hw_size = readl(&regs->ecc_range_ctrl) & SDRAM_ECC_RANGE_ADDR_MASK;
	hw_size += (1 << SDRAM_ECC_RANGE_ADDR_SHIFT);

	info->info.size = hw_size;
}

#ifdef CONFIG_ASPEED_ECC
static void ast2600_sdrammc_ecc_enable(struct dram_info *info)
{
	struct ast2600_sdrammc_regs *regs = info->regs;
	size_t conf_size;
	u32 reg;

	conf_size = CONFIG_ASPEED_ECC_SIZE * SDRAM_SIZE_1MB;
	if (conf_size > info->info.size) {
		printf("warning: ECC configured %dMB but actual size is %dMB\n",
		       CONFIG_ASPEED_ECC_SIZE,
		       info->info.size / SDRAM_SIZE_1MB);
		conf_size = info->info.size;
	} else if (conf_size == 0) {
		conf_size = info->info.size;
	}

	info->info.size = (((conf_size / 9) * 8) >> 20) << 20;
	writel(((info->info.size >> 20) - 1) << 20, &regs->ecc_range_ctrl);
	reg = readl(&regs->config) | SDRAM_CONF_ECC_SETUP;
	writel(reg, &regs->config);

	writel(0, &regs->test_init_val);
	writel(0x80000001, &regs->test_addr);
	writel(0x221, &regs->ecc_test_ctrl);
	while (0 == (readl(&regs->ecc_test_ctrl) & BIT(12)))
		;
	writel(0, &regs->ecc_test_ctrl);
	writel(BIT(31), &regs->intr_ctrl);
	writel(0, &regs->intr_ctrl);
}
#endif

static int ast2600_sdrammc_probe(struct udevice *dev)
{
	int ret;
	u32 reg;
	struct dram_info *priv = (struct dram_info *)dev_get_priv(dev);
	struct ast2600_sdrammc_regs *regs = priv->regs;
	struct udevice *clk_dev;

	/* find SCU base address from clock device */
	ret = uclass_get_device_by_driver(UCLASS_CLK,
					  DM_DRIVER_GET(aspeed_ast2600_scu), &clk_dev);
	if (ret) {
		debug("clock device not defined\n");
		return ret;
	}

	priv->scu = devfdt_get_addr_ptr(clk_dev);
	if (IS_ERR(priv->scu)) {
		debug("%s(): can't get SCU\n", __func__);
		return PTR_ERR(priv->scu);
	}

	if (readl(&priv->scu->dram_hdshk) & SCU_DRAM_HDSHK_RDY) {
		printf("already initialized, ");
		ast2600_sdrammc_update_size(priv);
		return 0;
	}

	reg = readl(&priv->scu->mpll);
	reg &= ~(SCU_PLL_BYPASS | SCU_PLL_OFF | SCU_PLL_DIV_MASK |
		 SCU_PLL_DENUM_MASK | SCU_PLL_NUM_MASK);
	reg |= (SCU_PLL_RST | SCU_MPLL_FREQ_CFG);
	writel(reg, &priv->scu->mpll);
	writel(SCU_MPLL_EXT_CFG, &priv->scu->mpll_ext);
	udelay(100);
	reg &= ~SCU_PLL_RST;
	writel(reg, &priv->scu->mpll);

	while ((readl(&priv->scu->mpll_ext) & BIT(31)) == 0)
		;

	ast2600_sdrammc_unlock(priv);
	ast2600_sdrammc_common_init(regs);
L_ast2600_sdramphy_train:
	ast2600_sdrammc_init_ddr4(priv);

	if (ast2600_sdramphy_check_status(priv) != 0) {
		printf("DDR4 PHY training fail, retrain\n");
		goto L_ast2600_sdramphy_train;
	}

	ast2600_sdrammc_calc_size(priv);

#ifndef CONFIG_ASPEED_BYPASS_SELFTEST
	if (ast2600_sdrammc_test(priv) != 0) {
		printf("%s: DDR4 init fail\n", __func__);
		return -EINVAL;
	}
#endif

#ifdef CONFIG_ASPEED_ECC
	ast2600_sdrammc_ecc_enable(priv);
#endif

	writel(readl(&priv->scu->dram_hdshk) | SCU_DRAM_HDSHK_RDY,
	       &priv->scu->dram_hdshk);

	clrbits_le32(&regs->intr_ctrl, MCR50_RESET_ALL_INTR);
	ast2600_sdrammc_lock(priv);
	return 0;
}

static int ast2600_sdrammc_of_to_plat(struct udevice *dev)
{
	struct dram_info *priv = dev_get_priv(dev);

	priv->regs = (void *)(uintptr_t)devfdt_get_addr_index(dev, 0);
	priv->phy_setting = (void *)(uintptr_t)devfdt_get_addr_index(dev, 1);
	priv->phy_status = (void *)(uintptr_t)devfdt_get_addr_index(dev, 2);

	priv->clock_rate = fdtdec_get_int(gd->fdt_blob, dev_of_offset(dev),
					  "clock-frequency", 0);
	if (!priv->clock_rate) {
		debug("DDR Clock Rate not defined\n");
		return -EINVAL;
	}

	return 0;
}

static int ast2600_sdrammc_get_info(struct udevice *dev, struct ram_info *info)
{
	struct dram_info *priv = dev_get_priv(dev);

	*info = priv->info;

	return 0;
}

static struct ram_ops ast2600_sdrammc_ops = {
	.get_info = ast2600_sdrammc_get_info,
};

static const struct udevice_id ast2600_sdrammc_ids[] = {
	{ .compatible = "aspeed,ast2600-sdrammc" },
	{ }
};

U_BOOT_DRIVER(sdrammc_ast2600) = {
	.name = "aspeed_ast2600_sdrammc",
	.id = UCLASS_RAM,
	.of_match = ast2600_sdrammc_ids,
	.ops = &ast2600_sdrammc_ops,
	.of_to_plat = ast2600_sdrammc_of_to_plat,
	.probe = ast2600_sdrammc_probe,
	.priv_auto = sizeof(struct dram_info),
};
