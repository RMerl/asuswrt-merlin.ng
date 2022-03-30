/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 */

#ifndef __BOARD_ENV_SPEC
#define __BOARD_ENV_SPEC

/* Board specific configuration */

/* KW40 */
#define MV_6710_DEV_ID			0x6710

#define MV_6710_Z1_REV			0x0
#define MV_6710_Z1_ID			((MV_6710_DEV_ID << 16) | MV_6710_Z1_REV)
#define MV_6710_Z1_NAME			"MV6710 Z1"

/* Armada XP Family */
#define MV_78130_DEV_ID			0x7813
#define MV_78160_DEV_ID			0x7816
#define MV_78230_DEV_ID			0x7823
#define MV_78260_DEV_ID			0x7826
#define MV_78460_DEV_ID			0x7846
#define MV_78000_DEV_ID			0x7888

#define MV_FPGA_DEV_ID			0x2107

#define MV_78XX0_Z1_REV			0x0

/* boards ID numbers */
#define BOARD_ID_BASE			0x0

/* New board ID numbers */
#define DB_88F78XX0_BP_ID		(BOARD_ID_BASE + 1)
#define RD_78460_SERVER_ID		(DB_88F78XX0_BP_ID + 1)
#define DB_78X60_PCAC_ID		(RD_78460_SERVER_ID + 1)
#define FPGA_88F78XX0_ID		(DB_78X60_PCAC_ID + 1)
#define DB_88F78XX0_BP_REV2_ID		(FPGA_88F78XX0_ID + 1)
#define RD_78460_NAS_ID			(DB_88F78XX0_BP_REV2_ID + 1)
#define DB_78X60_AMC_ID			(RD_78460_NAS_ID + 1)
#define DB_78X60_PCAC_REV2_ID		(DB_78X60_AMC_ID + 1)
#define RD_78460_SERVER_REV2_ID		(DB_78X60_PCAC_REV2_ID + 1)
#define DB_784MP_GP_ID			(RD_78460_SERVER_REV2_ID + 1)
#define RD_78460_CUSTOMER_ID		(DB_784MP_GP_ID + 1)
#define MV_MAX_BOARD_ID			(RD_78460_CUSTOMER_ID + 1)
#define INVALID_BOARD_ID		0xFFFFFFFF

/* Sample at Reset */
#define MPP_SAMPLE_AT_RESET(id)		(0x18230 + (id * 4))

/* BIOS Modes related defines */

#define SAR0_BOOTWIDTH_OFFSET		3
#define SAR0_BOOTWIDTH_MASK		(0x3 << SAR0_BOOTWIDTH_OFFSET)
#define SAR0_BOOTSRC_OFFSET		5
#define SAR0_BOOTSRC_MASK		(0xF << SAR0_BOOTSRC_OFFSET)

#define SAR0_L2_SIZE_OFFSET		19
#define SAR0_L2_SIZE_MASK		(0x3 << SAR0_L2_SIZE_OFFSET)
#define SAR0_CPU_FREQ_OFFSET		21
#define SAR0_CPU_FREQ_MASK		(0x7 << SAR0_CPU_FREQ_OFFSET)
#define SAR0_FABRIC_FREQ_OFFSET		24
#define SAR0_FABRIC_FREQ_MASK		(0xF << SAR0_FABRIC_FREQ_OFFSET)
#define SAR0_CPU0CORE_OFFSET		31
#define SAR0_CPU0CORE_MASK		(0x1 << SAR0_CPU0CORE_OFFSET)
#define SAR1_CPU0CORE_OFFSET		0
#define SAR1_CPU0CORE_MASK		(0x1 << SAR1_CPU0CORE_OFFSET)

#define PEX_CLK_100MHZ_OFFSET		2
#define PEX_CLK_100MHZ_MASK		(0x1 << PEX_CLK_100MHZ_OFFSET)

#define SAR1_FABRIC_MODE_OFFSET		19
#define SAR1_FABRIC_MODE_MASK		(0x1 << SAR1_FABRIC_MODE_OFFSET)
#define SAR1_CPU_MODE_OFFSET		20
#define SAR1_CPU_MODE_MASK		(0x1 << SAR1_CPU_MODE_OFFSET)

#define SAR_CPU_FAB_GET(cpu, fab)	(((cpu & 0x7) << 21) | ((fab & 0xF) << 24))


#define CORE_AVS_CONTROL_0REG		0x18300
#define CORE_AVS_CONTROL_2REG		0x18308
#define CPU_AVS_CONTROL2_REG		0x20868
#define CPU_AVS_CONTROL0_REG		0x20860
#define GENERAL_PURPOSE_RESERVED0_REG	0x182E0

#define MSAR_TCLK_OFFS			28
#define MSAR_TCLK_MASK			(0x1 << MSAR_TCLK_OFFS)


/* Controler environment registers offsets */
#define GEN_PURP_RES_1_REG		0x182F4
#define GEN_PURP_RES_2_REG		0x182F8

/* registers offsets */
#define MV_GPP_REGS_OFFSET(unit)	(0x18100 + ((unit) * 0x40))
#define MPP_CONTROL_REG(id)		(0x18000 + (id * 4))
#define MV_GPP_REGS_BASE(unit)		(MV_GPP_REGS_OFFSET(unit))
#define MV_GPP_REGS_BASE_0		(MV_GPP_REGS_OFFSET_0)

#define GPP_DATA_OUT_REG(grp)		(MV_GPP_REGS_BASE(grp) + 0x00)
#define GPP_DATA_OUT_REG_0		(MV_GPP_REGS_BASE_0 + 0x00)	/* Used in .S files */
#define GPP_DATA_OUT_EN_REG(grp)	(MV_GPP_REGS_BASE(grp) + 0x04)
#define GPP_BLINK_EN_REG(grp)		(MV_GPP_REGS_BASE(grp) + 0x08)
#define GPP_DATA_IN_POL_REG(grp)	(MV_GPP_REGS_BASE(grp) + 0x0C)
#define GPP_DATA_IN_REG(grp)		(MV_GPP_REGS_BASE(grp) + 0x10)
#define GPP_INT_CAUSE_REG(grp)		(MV_GPP_REGS_BASE(grp) + 0x14)
#define GPP_INT_MASK_REG(grp)		(MV_GPP_REGS_BASE(grp) + 0x18)
#define GPP_INT_LVL_REG(grp)		(MV_GPP_REGS_BASE(grp) + 0x1C)
#define GPP_OUT_SET_REG(grp)		(0x18130 + ((grp) * 0x40))
#define GPP_64_66_DATA_OUT_SET_REG	0x181A4
#define GPP_OUT_CLEAR_REG(grp)		(0x18134 + ((grp) * 0x40))
#define GPP_64_66_DATA_OUT_CLEAR_REG	0x181B0
#define GPP_FUNC_SELECT_REG		(MV_GPP_REGS_BASE(0) + 0x40)

#define MV_GPP66			(1 << 2)

/* Relevant for MV78XX0 */
#define GPP_DATA_OUT_SET_REG		(MV_GPP_REGS_BASE(0) + 0x20)
#define GPP_DATA_OUT_CLEAR_REG		(MV_GPP_REGS_BASE(0) + 0x24)

/* This define describes the maximum number of supported PEX Interfaces */
#define MV_PEX_MAX_IF			10
#define MV_PEX_MAX_UNIT			4

#define MV_SERDES_NUM_TO_PEX_NUM(num)	((num < 8) ? (num) : (8 + (num / 12)))

#define PEX_PHY_ACCESS_REG(unit)	(0x40000 + ((unit) % 2 * 0x40000) + \
					 ((unit)/2 * 0x2000) + 0x1B00)

#define SATA_BASE_REG(port)		(0xA2000 + (port)*0x2000)

#define SATA_PWR_PLL_CTRL_REG(port)	(SATA_BASE_REG(port) + 0x804)
#define SATA_DIG_LP_ENA_REG(port)	(SATA_BASE_REG(port) + 0x88C)
#define SATA_REF_CLK_SEL_REG(port)	(SATA_BASE_REG(port) + 0x918)
#define SATA_COMPHY_CTRL_REG(port)	(SATA_BASE_REG(port) + 0x920)
#define SATA_LP_PHY_EXT_CTRL_REG(port)	(SATA_BASE_REG(port) + 0x058)
#define SATA_LP_PHY_EXT_STAT_REG(port)	(SATA_BASE_REG(port) + 0x05C)
#define SATA_IMP_TX_SSC_CTRL_REG(port)	(SATA_BASE_REG(port) + 0x810)
#define SATA_GEN_1_SET_0_REG(port)	(SATA_BASE_REG(port) + 0x834)
#define SATA_GEN_1_SET_1_REG(port)	(SATA_BASE_REG(port) + 0x838)
#define SATA_GEN_2_SET_0_REG(port)	(SATA_BASE_REG(port) + 0x83C)
#define SATA_GEN_2_SET_1_REG(port)	(SATA_BASE_REG(port) + 0x840)

#define MV_ETH_BASE_ADDR		(0x72000)
#define MV_ETH_REGS_OFFSET(port)	(MV_ETH_BASE_ADDR - ((port) / 2) * \
					 0x40000 + ((port) % 2) * 0x4000)
#define MV_ETH_REGS_BASE(port)		MV_ETH_REGS_OFFSET(port)


#define SGMII_PWR_PLL_CTRL_REG(port)	(MV_ETH_REGS_BASE(port) + 0xE04)
#define SGMII_DIG_LP_ENA_REG(port)	(MV_ETH_REGS_BASE(port) + 0xE8C)
#define SGMII_REF_CLK_SEL_REG(port)	(MV_ETH_REGS_BASE(port) + 0xF18)
#define SGMII_SERDES_CFG_REG(port)	(MV_ETH_REGS_BASE(port) + 0x4A0)
#define SGMII_SERDES_STAT_REG(port)	(MV_ETH_REGS_BASE(port) + 0x4A4)
#define SGMII_COMPHY_CTRL_REG(port)	(MV_ETH_REGS_BASE(port) + 0xF20)
#define QSGMII_GEN_1_SETTING_REG(port)	(MV_ETH_REGS_BASE(port) + 0xE38)
#define QSGMII_SERDES_CFG_REG(port)	(MV_ETH_REGS_BASE(port) + 0x4a0)

#define SERDES_LINE_MUX_REG_0_7		0x18270
#define SERDES_LINE_MUX_REG_8_15	0x18274
#define QSGMII_CONTROL_1_REG		0x18404

/* SOC_CTRL_REG fields */
#define SCR_PEX_ENA_OFFS(pex)		((pex) & 0x3)
#define SCR_PEX_ENA_MASK(pex)		(1 << pex)

#define PCIE0_QUADX1_EN			(1<<7)
#define PCIE1_QUADX1_EN			(1<<8)

#define SCR_PEX_4BY1_OFFS(pex)		((pex) + 7)
#define SCR_PEX_4BY1_MASK(pex)		(1 << SCR_PEX_4BY1_OFFS(pex))

#define PCIE1_CLK_OUT_EN_OFF		5
#define PCIE1_CLK_OUT_EN_MASK		(1 << PCIE1_CLK_OUT_EN_OFF)

#define PCIE0_CLK_OUT_EN_OFF		4
#define PCIE0_CLK_OUT_EN_MASK		(1 << PCIE0_CLK_OUT_EN_OFF)

#define SCR_PEX0_4BY1_OFFS		7
#define SCR_PEX0_4BY1_MASK		(1 << SCR_PEX0_4BY1_OFFS)

#define SCR_PEX1_4BY1_OFFS		8
#define SCR_PEX1_4BY1_MASK		(1 << SCR_PEX1_4BY1_OFFS)


#define MV_MISC_REGS_OFFSET		(0x18200)
#define MV_MISC_REGS_BASE		(MV_MISC_REGS_OFFSET)
#define SOC_CTRL_REG			(MV_MISC_REGS_BASE + 0x4)

/*
 * PCI Express Control and Status Registers
 */
#define MAX_PEX_DEVICES			32
#define MAX_PEX_FUNCS			8
#define MAX_PEX_BUSSES			256

#define PXSR_PEX_BUS_NUM_OFFS		8	/* Bus Number Indication */
#define PXSR_PEX_BUS_NUM_MASK		(0xff << PXSR_PEX_BUS_NUM_OFFS)

#define PXSR_PEX_DEV_NUM_OFFS		16	/* Device Number Indication */
#define PXSR_PEX_DEV_NUM_MASK		(0x1f << PXSR_PEX_DEV_NUM_OFFS)

#define PXSR_DL_DOWN			0x1	/* DL_Down indication. */
#define PXCAR_CONFIG_EN			(1 << 31)
#define PEX_STATUS_AND_COMMAND		0x004
#define PXSAC_MABORT			(1 << 29) /* Recieved Master Abort */

/* PCI Express Configuration Address Register */

/* PEX_CFG_ADDR_REG (PXCAR) */
#define PXCAR_REG_NUM_OFFS		2
#define PXCAR_REG_NUM_MAX		0x3F
#define PXCAR_REG_NUM_MASK		(PXCAR_REG_NUM_MAX << PXCAR_REG_NUM_OFFS)
#define PXCAR_FUNC_NUM_OFFS		8
#define PXCAR_FUNC_NUM_MAX		0x7
#define PXCAR_FUNC_NUM_MASK		(PXCAR_FUNC_NUM_MAX << PXCAR_FUNC_NUM_OFFS)
#define PXCAR_DEVICE_NUM_OFFS		11
#define PXCAR_DEVICE_NUM_MAX		0x1F
#define PXCAR_DEVICE_NUM_MASK		(PXCAR_DEVICE_NUM_MAX << PXCAR_DEVICE_NUM_OFFS)
#define PXCAR_BUS_NUM_OFFS		16
#define PXCAR_BUS_NUM_MAX		0xFF
#define PXCAR_BUS_NUM_MASK		(PXCAR_BUS_NUM_MAX << PXCAR_BUS_NUM_OFFS)
#define PXCAR_EXT_REG_NUM_OFFS		24
#define PXCAR_EXT_REG_NUM_MAX		0xF

#define PXCAR_REAL_EXT_REG_NUM_OFFS     8
#define PXCAR_REAL_EXT_REG_NUM_MASK     (0xF << PXCAR_REAL_EXT_REG_NUM_OFFS)


#define PEX_CAPABILITIES_REG(if)	((MV_PEX_IF_REGS_BASE(if)) + 0x60)
#define PEX_LINK_CAPABILITIES_REG(if)	((MV_PEX_IF_REGS_BASE(if)) + 0x6C)
#define PEX_LINK_CTRL_STATUS_REG(if)	((MV_PEX_IF_REGS_BASE(if)) + 0x70)
#define PEX_LINK_CTRL_STATUS2_REG(if)	((MV_PEX_IF_REGS_BASE(if)) + 0x90)
#define PEX_CTRL_REG(if)		((MV_PEX_IF_REGS_BASE(if)) + 0x1A00)
#define PEX_STATUS_REG(if)		((MV_PEX_IF_REGS_BASE(if)) + 0x1A04)
#define PEX_COMPLT_TMEOUT_REG(if)	((MV_PEX_IF_REGS_BASE(if)) + 0x1A10)
#define PEX_PWR_MNG_EXT_REG(if)		((MV_PEX_IF_REGS_BASE(if)) + 0x1A18)
#define PEX_FLOW_CTRL_REG(if)		((MV_PEX_IF_REGS_BASE(if)) + 0x1A20)
#define PEX_DYNMC_WIDTH_MNG_REG(if)	((MV_PEX_IF_REGS_BASE(if)) + 0x1A30)
#define PEX_ROOT_CMPLX_SSPL_REG(if)	((MV_PEX_IF_REGS_BASE(if)) + 0x1A0C)
#define PEX_RAM_PARITY_CTRL_REG(if)	((MV_PEX_IF_REGS_BASE(if)) + 0x1A50)
#define PEX_DBG_CTRL_REG(if)		((MV_PEX_IF_REGS_BASE(if)) + 0x1A60)
#define PEX_DBG_STATUS_REG(if)		((MV_PEX_IF_REGS_BASE(if)) + 0x1A64)

#define PXLCSR_NEG_LNK_GEN_OFFS		16	/* Negotiated Link GEN */
#define PXLCSR_NEG_LNK_GEN_MASK		(0xf << PXLCSR_NEG_LNK_GEN_OFFS)
#define PXLCSR_NEG_LNK_GEN_1_1		(0x1 << PXLCSR_NEG_LNK_GEN_OFFS)
#define PXLCSR_NEG_LNK_GEN_2_0		(0x2 << PXLCSR_NEG_LNK_GEN_OFFS)

#define PEX_CFG_ADDR_REG(if)		((MV_PEX_IF_REGS_BASE(if)) + 0x18F8)
#define PEX_CFG_DATA_REG(if)		((MV_PEX_IF_REGS_BASE(if)) + 0x18FC)
#define PEX_CAUSE_REG(if)		((MV_PEX_IF_REGS_BASE(if)) + 0x1900)

#define PEX_CAPABILITY_REG		0x60
#define PEX_DEV_CAPABILITY_REG		0x64
#define PEX_DEV_CTRL_STAT_REG		0x68
#define PEX_LINK_CAPABILITY_REG		0x6C
#define PEX_LINK_CTRL_STAT_REG		0x70
#define PEX_LINK_CTRL_STAT_2_REG	0x90

#endif /* __BOARD_ENV_SPEC */
