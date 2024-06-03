// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 */

#include <common.h>
#include <spl.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>

#include "high_speed_env_spec.h"
#include "sys_env_lib.h"
#include "ctrl_pex.h"

#if defined(CONFIG_ARMADA_38X)
#elif defined(CONFIG_ARMADA_39X)
#else
#error "No device is defined"
#endif


/*
 * serdes_seq_db - holds all serdes sequences, their size and the
 * relevant index in the data array initialized in serdes_seq_init
 */
struct cfg_seq serdes_seq_db[SERDES_LAST_SEQ];

#define	SERDES_VERSION		"2.0"
#define ENDED_OK		"High speed PHY - Ended Successfully\n"

#define LINK_WAIT_CNTR		100
#define LINK_WAIT_SLEEP		100

#define MAX_UNIT_NUMB		4
#define TOPOLOGY_TEST_OK	0
#define WRONG_NUMBER_OF_UNITS	1
#define SERDES_ALREADY_IN_USE	2
#define UNIT_NUMBER_VIOLATION	3

/*
 * serdes_lane_in_use_count contains the exact amount of serdes lanes
 * needed per type
 */
u8 serdes_lane_in_use_count[MAX_UNITS_ID][MAX_UNIT_NUMB] = {
	/* 0  1  2  3  */
	{  1, 1, 1, 1 },	/* PEX     */
	{  1, 1, 1, 1 },	/* ETH_GIG */
	{  1, 1, 0, 0 },	/* USB3H   */
	{  1, 1, 1, 0 },	/* USB3D   */
	{  1, 1, 1, 1 },	/* SATA    */
	{  1, 0, 0, 0 },	/* QSGMII  */
	{  4, 0, 0, 0 },	/* XAUI    */
	{  2, 0, 0, 0 }		/* RXAUI   */
};

/*
 * serdes_unit_count count unit number.
 * (i.e a single XAUI is counted as 1 unit)
 */
u8 serdes_unit_count[MAX_UNITS_ID] = { 0 };

/* Selector mapping for A380-A0 and A390-Z1 */
u8 selectors_serdes_rev2_map[LAST_SERDES_TYPE][MAX_SERDES_LANES] = {
	/* 0      1      2       3       4       5       6 */
	{ 0x1,   0x1,    NA,	 NA,	 NA,	 NA,     NA  }, /* PEX0 */
	{ NA,    NA,     0x1,	 NA,	 0x1,	 NA,     0x1 }, /* PEX1 */
	{ NA,    NA,     NA,	 NA,	 0x7,	 0x1,    NA  }, /* PEX2 */
	{ NA,    NA,     NA,	 0x1,	 NA,	 NA,     NA  }, /* PEX3 */
	{ 0x2,   0x3,    NA,	 NA,	 NA,	 NA,     NA  }, /* SATA0 */
	{ NA,    NA,     0x3,	 NA,	 NA,	 NA,     NA  }, /* SATA1 */
	{ NA,    NA,     NA,	 NA,	 0x6,	 0x2,    NA  }, /* SATA2 */
	{ NA,	 NA,     NA,	 0x3,	 NA,	 NA,     NA  }, /* SATA3 */
	{ 0x3,   0x4,    NA,     NA,	 NA,	 NA,     NA  }, /* SGMII0 */
	{ NA,    0x5,    0x4,    NA,	 0x3,	 NA,     NA  }, /* SGMII1 */
	{ NA,    NA,     NA,	 0x4,	 NA,	 0x3,    NA  }, /* SGMII2 */
	{ NA,    0x7,    NA,	 NA,	 NA,	 NA,     NA  }, /* QSGMII */
	{ NA,    0x6,    NA,	 NA,	 0x4,	 NA,     NA  }, /* USB3_HOST0 */
	{ NA,    NA,     NA,	 0x5,	 NA,	 0x4,    NA  }, /* USB3_HOST1 */
	{ NA,    NA,     NA,	 0x6,	 0x5,	 0x5,    NA  }, /* USB3_DEVICE */
#ifdef CONFIG_ARMADA_39X
	{ NA,    NA,     0x5,	 NA,	 0x8,	 NA,     0x2 }, /* SGMII3 */
	{ NA,    NA,     NA,	 0x8,	 0x9,	 0x8,    0x4 }, /* XAUI */
	{ NA,    NA,     NA,	 NA,	 NA,	 0x8,    0x4 }, /* RXAUI */
#endif
	{ 0x0,   0x0,    0x0,	 0x0,	 0x0,	 0x0,    NA  }  /* DEFAULT_SERDES */
};

/* Selector mapping for PEX by 4 confiuration */
u8 common_phys_selectors_pex_by4_lanes[] = { 0x1, 0x2, 0x2, 0x2 };

static const char *const serdes_type_to_string[] = {
	"PCIe0",
	"PCIe1",
	"PCIe2",
	"PCIe3",
	"SATA0",
	"SATA1",
	"SATA2",
	"SATA3",
	"SGMII0",
	"SGMII1",
	"SGMII2",
	"QSGMII",
	"USB3 HOST0",
	"USB3 HOST1",
	"USB3 DEVICE",
	"SGMII3",
	"XAUI",
	"RXAUI",
	"DEFAULT SERDES",
	"LAST_SERDES_TYPE"
};

struct serdes_unit_data {
	u8 serdes_unit_id;
	u8 serdes_unit_num;
};

static struct serdes_unit_data serdes_type_to_unit_info[] = {
	{PEX_UNIT_ID, 0,},
	{PEX_UNIT_ID, 1,},
	{PEX_UNIT_ID, 2,},
	{PEX_UNIT_ID, 3,},
	{SATA_UNIT_ID, 0,},
	{SATA_UNIT_ID, 1,},
	{SATA_UNIT_ID, 2,},
	{SATA_UNIT_ID, 3,},
	{ETH_GIG_UNIT_ID, 0,},
	{ETH_GIG_UNIT_ID, 1,},
	{ETH_GIG_UNIT_ID, 2,},
	{QSGMII_UNIT_ID, 0,},
	{USB3H_UNIT_ID, 0,},
	{USB3H_UNIT_ID, 1,},
	{USB3D_UNIT_ID, 0,},
	{ETH_GIG_UNIT_ID, 3,},
	{XAUI_UNIT_ID, 0,},
	{RXAUI_UNIT_ID, 0,},
};

/* Sequences DB */

/*
 * SATA and SGMII
 */

struct op_params sata_port0_power_up_params[] = {
	/*
	 * unit_base_reg, unit_offset, mask, SATA data, wait_time,
	 * num_of_loops
	 */
	/* Access to reg 0x48(OOB param 1) */
	{SATA_VENDOR_PORT_0_REG_ADDR, 0x38000, 0xffffffff, {0x48,}, 0, 0},
	/* OOB Com_wake and Com_reset spacing upper limit data */
	{SATA_VENDOR_PORT_0_REG_DATA, 0x38000, 0xf03f, {0x6018,}, 0, 0},
	/* Access to reg 0xa(PHY Control) */
	{SATA_VENDOR_PORT_0_REG_ADDR, 0x38000, 0xffffffff, {0xa,}, 0, 0},
	/* Rx clk and Tx clk select non-inverted mode */
	{SATA_VENDOR_PORT_0_REG_DATA, 0x38000, 0x3000, {0x0,}, 0, 0},
	/* Power Down Sata addr */
	{SATA_CTRL_REG_IND_ADDR, 0x38000, 0xffffffff, {0x0,}, 0, 0},
	/* Power Down Sata Port 0 */
	{SATA_CTRL_REG_IND_DATA, 0x38000, 0xffff00ff, {0xc40040,}, 0, 0},
};

struct op_params sata_port1_power_up_params[] = {
	/*
	 * unit_base_reg, unit_offset, mask, SATA data, wait_time,
	 * num_of_loops
	 */
	/* Access to reg 0x48(OOB param 1) */
	{SATA_VENDOR_PORT_1_REG_ADDR, 0x38000, 0xffffffff, {0x48,}, 0, 0},
	/* OOB Com_wake and Com_reset spacing upper limit data */
	{SATA_VENDOR_PORT_1_REG_DATA, 0x38000, 0xf03f, {0x6018,}, 0, 0},
	/* Access to reg 0xa(PHY Control) */
	{SATA_VENDOR_PORT_1_REG_ADDR, 0x38000, 0xffffffff, {0xa,}, 0, 0},
	/* Rx clk and Tx clk select non-inverted mode */
	{SATA_VENDOR_PORT_1_REG_DATA, 0x38000, 0x3000, {0x0,}, 0, 0},
	/* Power Down Sata addr */
	{SATA_CTRL_REG_IND_ADDR, 0x38000, 0xffffffff, {0x0,}, 0, 0},
	/* Power Down Sata Port 1 */
	{SATA_CTRL_REG_IND_DATA, 0x38000, 0xffffff00, {0xc44000,}, 0, 0},
};

/* SATA and SGMII - power up seq */
struct op_params sata_and_sgmii_power_up_params[] = {
	/*
	 * unit_base_reg, unit_offset, mask, SATA data, SGMII data,
	 * wait_time, num_of_loops
	 */
	/* Power Up */
	{COMMON_PHY_CONFIGURATION1_REG, 0x28, 0x90006, {0x80002, 0x80002},
	 0, 0},
	/* Unreset */
	{COMMON_PHY_CONFIGURATION1_REG, 0x28, 0x7800, {0x6000, 0x6000}, 0, 0},
	/* Phy Selector */
	{POWER_AND_PLL_CTRL_REG, 0x800, 0x0e0, {0x0, 0x80}, 0, 0},
	/* Ref clock source select */
	{MISC_REG, 0x800, 0x440, {0x440, 0x400}, 0, 0}
};

/* SATA and SGMII - speed config seq */
struct op_params sata_and_sgmii_speed_config_params[] = {
	/*
	 * unit_base_reg, unit_offset, mask, SATA data,
	 * SGMII (1.25G), SGMII (3.125G), wait_time, num_of_loops
	 */
	/* Baud Rate */
	{COMMON_PHY_CONFIGURATION1_REG, 0x28, 0x3fc00000,
	 {0x8800000, 0x19800000, 0x22000000}, 0, 0},
	/* Select Baud Rate for SATA only */
	{INTERFACE_REG, 0x800, 0xc00, {0x800, NO_DATA, NO_DATA}, 0, 0},
	/* Phy Gen RX and TX */
	{ISOLATE_REG, 0x800, 0xff, {NO_DATA, 0x66, 0x66}, 0, 0},
	/* Bus Width */
	{LOOPBACK_REG, 0x800, 0xe, {0x4, 0x2, 0x2}, 0, 0}
};

/* SATA and SGMII - TX config seq */
struct op_params sata_and_sgmii_tx_config_params1[] = {
	/*
	 * unitunit_base_reg, unit_offset, mask, SATA data, SGMII data,
	 * wait_time, num_of_loops
	 */
	{GLUE_REG, 0x800, 0x1800, {NO_DATA, 0x800}, 0, 0},
	/* Sft Reset pulse */
	{RESET_DFE_REG, 0x800, 0x401, {0x401, 0x401}, 0, 0},
	/* Sft Reset pulse */
	{RESET_DFE_REG, 0x800, 0x401, {0x0, 0x0}, 0, 0},
	/* Power up PLL, RX and TX */
	{COMMON_PHY_CONFIGURATION1_REG, 0x28, 0xf0000, {0x70000, 0x70000},
	 0, 0}
};

struct op_params sata_port0_tx_config_params[] = {
	/*
	 * unit_base_reg, unit_offset, mask, SATA data, wait_time,
	 * num_of_loops
	 */
	/* Power Down Sata addr */
	{SATA_CTRL_REG_IND_ADDR, 0x38000, 0xffffffff, {0x0}, 0, 0},
	/* Power Down Sata  Port 0 */
	{SATA_CTRL_REG_IND_DATA, 0x38000, 0xffff00ff, {0xc40000}, 0, 0},
	/* Regret bit addr */
	{SATA_CTRL_REG_IND_ADDR, 0x38000, 0xffffffff, {0x4}, 0, 0},
	/* Regret bit data */
	{SATA_CTRL_REG_IND_DATA, 0x38000, 0xffffffff, {0x80}, 0, 0}
};

struct op_params sata_port1_tx_config_params[] = {
	/*
	 * unit_base_reg, unit_offset, mask, SATA data, wait_time,
	 * num_of_loops
	 */
	/* Power Down Sata addr */
	{SATA_CTRL_REG_IND_ADDR, 0x38000, 0xffffffff, {0x0}, 0, 0},
	/* Power Down Sata Port 1 */
	{SATA_CTRL_REG_IND_DATA, 0x38000, 0xffffff00, {0xc40000}, 0, 0},
	/* Regret bit addr */
	{SATA_CTRL_REG_IND_ADDR, 0x38000, 0xffffffff, {0x4}, 0, 0},
	/* Regret bit data */
	{SATA_CTRL_REG_IND_DATA, 0x38000, 0xffffffff, {0x80}, 0, 0}
};

struct op_params sata_and_sgmii_tx_config_serdes_rev1_params2[] = {
	/*
	 * unit_base_reg, unit_offset, mask, SATA data, SGMII data,
	 * wait_time, num_of_loops
	 */
	/* Wait for PHY power up sequence to finish */
	{COMMON_PHY_STATUS1_REG, 0x28, 0xc, {0xc, 0xc}, 10, 1000},
	/* Wait for PHY power up sequence to finish */
	{COMMON_PHY_STATUS1_REG, 0x28, 0x1, {0x1, 0x1}, 1, 1000}
};

struct op_params sata_and_sgmii_tx_config_serdes_rev2_params2[] = {
	/*
	 * unit_base_reg, unit_offset, mask, SATA data, SGMII data,
	 * wait_time, num_of_loops
	 */
	/* Wait for PHY power up sequence to finish */
	{COMMON_PHY_STATUS1_REG, 0x28, 0xc, {0xc, 0xc}, 10, 1000},
	/* Assert Rx Init for SGMII */
	{COMMON_PHY_CONFIGURATION1_REG, 0x28, 0x40000000, {NA, 0x40000000},
	 0, 0},
	/* Assert Rx Init for SATA */
	{ISOLATE_REG, 0x800, 0x400, {0x400, NA}, 0, 0},
	/* Wait for PHY power up sequence to finish */
	{COMMON_PHY_STATUS1_REG, 0x28, 0x1, {0x1, 0x1}, 1, 1000},
	/* De-assert Rx Init for SGMII */
	{COMMON_PHY_CONFIGURATION1_REG, 0x28, 0x40000000, {NA, 0x0}, 0, 0},
	/* De-assert Rx Init for SATA */
	{ISOLATE_REG, 0x800, 0x400, {0x0, NA}, 0, 0},
	/* os_ph_offset_force (align 90) */
	{RX_REG3, 0x800, 0xff, {0xde, NO_DATA}, 0, 0},
	/* Set os_ph_valid */
	{RX_REG3, 0x800, 0x100, {0x100, NO_DATA}, 0, 0},
	/* Unset os_ph_valid */
	{RX_REG3, 0x800, 0x100, {0x0, NO_DATA}, 0, 0},
};

struct op_params sata_electrical_config_serdes_rev1_params[] = {
	/*
	 * unit_base_reg, unit_offset, mask, SATA data, wait_time,
	 * num_of_loops
	 */
	/* enable SSC and DFE update enable */
	{COMMON_PHY_CONFIGURATION4_REG, 0x28, 0x400008, {0x400000,}, 0, 0},
	/* tximpcal_th and rximpcal_th */
	{VTHIMPCAL_CTRL_REG, 0x800, 0xff00, {0x4000,}, 0, 0},
	/* SQ_THRESH and FFE Setting */
	{SQUELCH_FFE_SETTING_REG, 0x800, 0xfff, {0x6cf,}, 0, 0},
	/* G1_TX SLEW, EMPH1 and AMP */
	{G1_SETTINGS_0_REG, 0x800, 0xffff, {0x8a32,}, 0, 0},
	/* G1_RX SELMUFF, SELMUFI, SELMUPF and SELMUPI */
	{G1_SETTINGS_1_REG, 0x800, 0x3ff, {0x3c9,}, 0, 0},
	/* G2_TX SLEW, EMPH1 and AMP */
	{G2_SETTINGS_0_REG, 0x800, 0xffff, {0x8b5c,}, 0, 0},
	/* G2_RX SELMUFF, SELMUFI, SELMUPF and SELMUPI */
	{G2_SETTINGS_1_REG, 0x800, 0x3ff, {0x3d2,}, 0, 0},
	/* G3_TX SLEW, EMPH1 and AMP */
	{G3_SETTINGS_0_REG, 0x800, 0xffff, {0xe6e,}, 0, 0},
	/* G3_RX SELMUFF, SELMUFI, SELMUPF and SELMUPI */
	{G3_SETTINGS_1_REG, 0x800, 0x3ff, {0x3d2,}, 0, 0},
	/* Cal rxclkalign90 ext enable and Cal os ph ext */
	{CAL_REG6, 0x800, 0xff00, {0xdd00,}, 0, 0},
	/* Dtl Clamping disable and Dtl clamping Sel(6000ppm) */
	{RX_REG2, 0x800, 0xf0, {0x70,}, 0, 0},
};

struct op_params sata_electrical_config_serdes_rev2_params[] = {
	/*
	 * unit_base_reg, unit_offset, mask, SATA data, wait_time,
	 * num_of_loops
	 */
	/* SQ_THRESH and FFE Setting */
	{SQUELCH_FFE_SETTING_REG, 0x800, 0xf00, {0x600}, 0, 0},
	/* enable SSC and DFE update enable */
	{COMMON_PHY_CONFIGURATION4_REG, 0x28, 0x400008, {0x400000}, 0, 0},
	/* G1_TX SLEW, EMPH1 and AMP */
	{G1_SETTINGS_0_REG, 0x800, 0xffff, {0x8a32}, 0, 0},
	/* G1_RX SELMUFF, SELMUFI, SELMUPF and SELMUPI */
	{G1_SETTINGS_1_REG, 0x800, 0x3ff, {0x3c9}, 0, 0},
	/* G2_TX SLEW, EMPH1 and AMP */
	{G2_SETTINGS_0_REG, 0x800, 0xffff, {0x8b5c}, 0, 0},
	/* G2_RX SELMUFF, SELMUFI, SELMUPF and SELMUPI */
	{G2_SETTINGS_1_REG, 0x800, 0x3ff, {0x3d2}, 0, 0},
	/* G3_TX SLEW, EMPH1 and AMP */
	{G3_SETTINGS_0_REG, 0x800, 0xffff, {0xe6e}, 0, 0},
	/*
	 * G3_RX SELMUFF, SELMUFI, SELMUPF and SELMUPI & DFE_En Gen3,
	 * DC wander calibration dis
	 */
	{G3_SETTINGS_1_REG, 0x800, 0x47ff, {0x7d2}, 0, 0},
	/* Bit[12]=0x0 idle_sync_en */
	{PCIE_REG0, 0x800, 0x1000, {0x0}, 0, 0},
	/* Dtl Clamping disable and Dtl clamping Sel(6000ppm) */
	{RX_REG2, 0x800, 0xf0, {0x70,}, 0, 0},
	/* tximpcal_th and rximpcal_th */
	{VTHIMPCAL_CTRL_REG, 0x800, 0xff00, {0x3000}, 0, 0},
	/* DFE_STEP_FINE_FX[3:0] =0xa */
	{DFE_REG0, 0x800, 0xa00f, {0x800a}, 0, 0},
	/* DFE_EN and Dis Update control from pin disable */
	{DFE_REG3, 0x800, 0xc000, {0x0}, 0, 0},
	/* FFE Force FFE_REs and cap settings for Gen1 */
	{G1_SETTINGS_3_REG, 0x800, 0xff, {0xcf}, 0, 0},
	/* FFE Force FFE_REs and cap settings for Gen2 */
	{G2_SETTINGS_3_REG, 0x800, 0xff, {0xbf}, 0, 0},
	/* FE Force FFE_REs=4 and cap settings for Gen3n */
	{G3_SETTINGS_3_REG, 0x800, 0xff, {0xcf}, 0, 0},
	/* Set DFE Gen 3 Resolution to 3 */
	{G3_SETTINGS_4_REG, 0x800, 0x300, {0x300}, 0, 0},
};

struct op_params sgmii_electrical_config_serdes_rev1_params[] = {
	/*
	 * unit_base_reg, unit_offset, mask, SGMII (1.25G), SGMII (3.125G),
	 * wait_time, num_of_loops
	 */
	/* G1_RX SELMUFF, SELMUFI, SELMUPF and SELMUPI */
	{G1_SETTINGS_1_REG, 0x800, 0x3ff, {0x3c9, 0x3c9}, 0, 0},
	/* SQ_THRESH and FFE Setting */
	{SQUELCH_FFE_SETTING_REG, 0x800, 0xfff, {0x8f, 0xbf}, 0, 0},
	/* tximpcal_th and rximpcal_th */
	{VTHIMPCAL_CTRL_REG, 0x800, 0xff00, {0x4000, 0x4000}, 0, 0},
};

struct op_params sgmii_electrical_config_serdes_rev2_params[] = {
	/*
	 * unit_base_reg, unit_offset, mask, SGMII (1.25G), SGMII (3.125G),
	 * wait_time, num_of_loops
	 */
	/* Set Slew_rate, Emph and Amp */
	{G1_SETTINGS_0_REG, 0x800, 0xffff, {0x8fa, 0x8fa}, 0, 0},
	/* G1_RX SELMUFF, SELMUFI, SELMUPF and SELMUPI */
	{G1_SETTINGS_1_REG, 0x800, 0x3ff, {0x3c9, 0x3c9}, 0, 0},
	/* DTL_FLOOP_EN */
	{RX_REG2, 0x800, 0x4, {0x0, 0x0}, 0, 0},
	/* G1 FFE Setting Force, RES and CAP */
	{G1_SETTINGS_3_REG, 0x800, 0xff, {0x8f, 0xbf}, 0, 0},
	/* tximpcal_th and rximpcal_th */
	{VTHIMPCAL_CTRL_REG, 0x800, 0xff00, {0x3000, 0x3000}, 0, 0},
};

/*
 * PEX and USB3
 */

/* PEX and USB3 - power up seq for Serdes Rev 1.2 */
struct op_params pex_and_usb3_power_up_serdes_rev1_params[] = {
	/*
	 * unit_base_reg, unit_offset, mask, PEX data, USB3 data,
	 * wait_time, num_of_loops
	 */
	{COMMON_PHY_CONFIGURATION1_REG, 0x28, 0x3fc7f806,
	 {0x4471804, 0x4479804}, 0, 0},
	{COMMON_PHY_CONFIGURATION2_REG, 0x28, 0x5c, {0x58, 0x58}, 0, 0},
	{COMMON_PHY_CONFIGURATION4_REG, 0x28, 0x3, {0x1, 0x1}, 0, 0},
	{COMMON_PHY_CONFIGURATION1_REG, 0x28, 0x7800, {0x6000, 0xe000}, 0, 0},
	{GLOBAL_CLK_CTRL, 0x800, 0xd, {0x5, 0x1}, 0, 0},
	/* Ref clock source select */
	{MISC_REG, 0x800, 0x4c0, {0x80, 0x4c0}, 0, 0}
};

/* PEX and USB3 - power up seq for Serdes Rev 2.1 */
struct op_params pex_and_usb3_power_up_serdes_rev2_params[] = {
	/*
	 * unit_base_reg, unit_offset, mask, PEX data, USB3 data,
	 * wait_time, num_of_loops
	 */
	{COMMON_PHY_CONFIGURATION1_REG, 0x28, 0x3fc7f806,
	 {0x4471804, 0x4479804}, 0, 0},
	{COMMON_PHY_CONFIGURATION2_REG, 0x28, 0x5c, {0x58, 0x58}, 0, 0},
	{COMMON_PHY_CONFIGURATION4_REG, 0x28, 0x3, {0x1, 0x1}, 0, 0},
	{COMMON_PHY_CONFIGURATION1_REG, 0x28, 0x7800, {0x6000, 0xe000}, 0, 0},
	{GLOBAL_CLK_CTRL, 0x800, 0xd, {0x5, 0x1}, 0, 0},
	{GLOBAL_MISC_CTRL, 0x800, 0xc0, {0x0, NO_DATA}, 0, 0},
	/* Ref clock source select */
	{MISC_REG, 0x800, 0x4c0, {0x80, 0x4c0}, 0, 0}
};

/* PEX and USB3 - speed config seq */
struct op_params pex_and_usb3_speed_config_params[] = {
	/*
	 * unit_base_reg, unit_offset, mask, PEX data, USB3 data,
	 * wait_time, num_of_loops
	 */
	/* Maximal PHY Generation Setting */
	{INTERFACE_REG, 0x800, 0xc00, {0x400, 0x400, 0x400, 0x400, 0x400},
	 0, 0},
};

struct op_params usb3_electrical_config_serdes_rev1_params[] = {
	/* Spread Spectrum Clock Enable */
	{LANE_CFG4_REG, 0x800, 0x80, {0x80}, 0, 0},
	/* G2_TX_SSC_AMP[6:0]=4.5k_p_pM and TX emphasis mode=m_v */
	{G2_SETTINGS_2_REG, 0x800, 0xfe40, {0x4440}, 0, 0},
	/* tximpcal_th and rximpcal_th */
	{VTHIMPCAL_CTRL_REG, 0x800, 0xff00, {0x4000}, 0, 0},
	/* G2_RX SELMUFF, SELMUFI, SELMUPF and SELMUPI */
	{G2_SETTINGS_1_REG, 0x800, 0x3ff, {0x3d2}, 0, 0},
	/* FFE Setting Force, RES and CAP */
	{SQUELCH_FFE_SETTING_REG, 0x800, 0xff, {0xef}, 0, 0},
	/* Dtl Clamping disable and Dtl-clamping-Sel(6000ppm) */
	{RX_REG2, 0x800, 0xf0, {0x70}, 0, 0},
	/* cal_rxclkalign90_ext_en and cal_os_ph_ext */
	{CAL_REG6, 0x800, 0xff00, {0xd500}, 0, 0},
	/* vco_cal_vth_sel */
	{REF_REG0, 0x800, 0x38, {0x20}, 0, 0},
};

struct op_params usb3_electrical_config_serdes_rev2_params[] = {
	/* Spread Spectrum Clock Enable */
	{LANE_CFG4_REG, 0x800, 0x80, {0x80}, 0, 0},
	/* G2_TX_SSC_AMP[6:0]=4.5k_p_pM and TX emphasis mode=m_v */
	{G2_SETTINGS_2_REG, 0x800, 0xfe40, {0x4440}, 0, 0},
	/* G2_RX SELMUFF, SELMUFI, SELMUPF and SELMUPI */
	{G2_SETTINGS_1_REG, 0x800, 0x3ff, {0x3d2}, 0, 0},
	/* Dtl Clamping disable and Dtl-clamping-Sel(6000ppm) */
	{RX_REG2, 0x800, 0xf0, {0x70}, 0, 0},
	/* vco_cal_vth_sel */
	{REF_REG0, 0x800, 0x38, {0x20}, 0, 0},
	/* Spread Spectrum Clock Enable */
	{LANE_CFG5_REG, 0x800, 0x4, {0x4}, 0, 0},
};

/* PEX and USB3 - TX config seq */

/*
 * For PEXx1: the pex_and_usb3_tx_config_params1/2/3 configurations should run
 *            one by one on the lane.
 * For PEXx4: the pex_and_usb3_tx_config_params1/2/3 configurations should run
 *            by setting each sequence for all 4 lanes.
 */
struct op_params pex_and_usb3_tx_config_params1[] = {
	/*
	 * unit_base_reg, unit_offset, mask, PEX data, USB3 data,
	 * wait_time, num_of_loops
	 */
	{GLOBAL_CLK_CTRL, 0x800, 0x1, {0x0, 0x0}, 0, 0},
	/* 10ms delay */
	{0x0, 0x0, 0x0, {0x0, 0x0}, 10, 0},
	/* os_ph_offset_force (align 90) */
	{RX_REG3, 0x800, 0xff, {0xdc, NO_DATA}, 0, 0},
	/* Set os_ph_valid */
	{RX_REG3, 0x800, 0x100, {0x100, NO_DATA}, 0, 0},
	/* Unset os_ph_valid */
	{RX_REG3, 0x800, 0x100, {0x0, NO_DATA}, 0, 0},
};

struct op_params pex_and_usb3_tx_config_params2[] = {
	/*
	 * unit_base_reg, unit_offset, mask, PEX data, USB3 data,
	 * wait_time, num_of_loops
	 */
	/* Sft Reset pulse */
	{RESET_DFE_REG, 0x800, 0x401, {0x401, 0x401}, 0, 0},
};

struct op_params pex_and_usb3_tx_config_params3[] = {
	/*
	 * unit_base_reg, unit_offset, mask, PEX data, USB3 data,
	 * wait_time, num_of_loops
	 */
	/* Sft Reset pulse */
	{RESET_DFE_REG, 0x800, 0x401, {0x0, 0x0}, 0, 0},
	/* 10ms delay */
	{0x0, 0x0, 0x0, {0x0, 0x0}, 10, 0}
};

/* PEX by 4 config seq */
struct op_params pex_by4_config_params[] = {
	/* unit_base_reg, unit_offset, mask, data, wait_time, num_of_loops */
	{GLOBAL_CLK_SRC_HI, 0x800, 0x7, {0x5, 0x0, 0x0, 0x2}, 0, 0},
	/* Lane Alignement enable */
	{LANE_ALIGN_REG0, 0x800, 0x1000, {0x0, 0x0, 0x0, 0x0}, 0, 0},
	/* Max PLL phy config */
	{CALIBRATION_CTRL_REG, 0x800, 0x1000, {0x1000, 0x1000, 0x1000, 0x1000},
	 0, 0},
	/* Max PLL pipe config */
	{LANE_CFG1_REG, 0x800, 0x600, {0x600, 0x600, 0x600, 0x600}, 0, 0},
};

/* USB3 device donfig seq */
struct op_params usb3_device_config_params[] = {
	/* unit_base_reg, unit_offset, mask, data, wait_time, num_of_loops */
	{LANE_CFG4_REG, 0x800, 0x200, {0x200}, 0, 0}
};

/* PEX - electrical configuration seq Rev 1.2 */
struct op_params pex_electrical_config_serdes_rev1_params[] = {
	/*
	 * unit_base_reg, unit_offset, mask, PEX data, wait_time,
	 * num_of_loops
	 */
	/* G1_TX_SLEW_CTRL_EN and G1_TX_SLEW_RATE */
	{G1_SETTINGS_0_REG, 0x800, 0xf000, {0xb000}, 0, 0},
	/* G1_RX SELMUFF, SELMUFI, SELMUPF and SELMUPI */
	{G1_SETTINGS_1_REG, 0x800, 0x3ff, {0x3c9}, 0, 0},
	/* G2_RX SELMUFF, SELMUFI, SELMUPF and SELMUPI */
	{G2_SETTINGS_1_REG, 0x800, 0x3ff, {0x3c9}, 0, 0},
	/* CFG_DFE_EN_SEL */
	{LANE_CFG4_REG, 0x800, 0x8, {0x8}, 0, 0},
	/* FFE Setting Force, RES and CAP */
	{SQUELCH_FFE_SETTING_REG, 0x800, 0xff, {0xaf}, 0, 0},
	/* tximpcal_th and rximpcal_th */
	{VTHIMPCAL_CTRL_REG, 0x800, 0xff00, {0x3000}, 0, 0},
	/* cal_rxclkalign90_ext_en and cal_os_ph_ext */
	{CAL_REG6, 0x800, 0xff00, {0xdc00}, 0, 0},
};

/* PEX - electrical configuration seq Rev 2.1 */
struct op_params pex_electrical_config_serdes_rev2_params[] = {
	/*
	 * unit_base_reg, unit_offset, mask, PEX data, wait_time,
	 * num_of_loops
	 */
	/* G1_TX_SLEW_CTRL_EN and G1_TX_SLEW_RATE */
	{G1_SETTINGS_0_REG, 0x800, 0xf000, {0xb000}, 0, 0},
	/* G1_RX SELMUFF, SELMUFI, SELMUPF and SELMUPI */
	{G1_SETTINGS_1_REG, 0x800, 0x3ff, {0x3c9}, 0, 0},
	/* G1 FFE Setting Force, RES and CAP */
	{G1_SETTINGS_3_REG, 0x800, 0xff, {0xcf}, 0, 0},
	/* G2_RX SELMUFF, SELMUFI, SELMUPF and SELMUPI */
	{G2_SETTINGS_1_REG, 0x800, 0x3ff, {0x3c9}, 0, 0},
	/* G2 FFE Setting Force, RES and CAP */
	{G2_SETTINGS_3_REG, 0x800, 0xff, {0xaf}, 0, 0},
	/* G2 DFE resolution value */
	{G2_SETTINGS_4_REG, 0x800, 0x300, {0x300}, 0, 0},
	/* DFE resolution force */
	{DFE_REG0, 0x800, 0x8000, {0x8000}, 0, 0},
	/* Tx amplitude for Tx Margin 0 */
	{PCIE_REG1, 0x800, 0xf80, {0xd00}, 0, 0},
	/* Tx_Emph value for -3.5d_b and -6d_b */
	{PCIE_REG3, 0x800, 0xff00, {0xaf00}, 0, 0},
	/* CFG_DFE_EN_SEL */
	{LANE_CFG4_REG, 0x800, 0x8, {0x8}, 0, 0},
	/* tximpcal_th and rximpcal_th */
	{VTHIMPCAL_CTRL_REG, 0x800, 0xff00, {0x3000}, 0, 0},
	/* Force receiver detected */
	{LANE_CFG0_REG, 0x800, 0x8000, {0x8000}, 0, 0},
};

/* PEX - configuration seq for REF_CLOCK_25MHz */
struct op_params pex_config_ref_clock25_m_hz[] = {
	/*
	 * unit_base_reg, unit_offset, mask, PEX data, wait_time,
	 * num_of_loops
	 */
	/* Bits[4:0]=0x2 - REF_FREF_SEL */
	{POWER_AND_PLL_CTRL_REG, 0x800, 0x1f, {0x2}, 0, 0},
	/* Bit[10]=0x1   - REFCLK_SEL */
	{MISC_REG, 0x800, 0x400, {0x400}, 0, 0},
	/* Bits[7:0]=0x7 - CFG_PM_RXDLOZ_WAIT */
	{GLOBAL_PM_CTRL, 0x800, 0xff, {0x7}, 0, 0},
};

/* PEX - configuration seq for REF_CLOCK_40MHz */
struct op_params pex_config_ref_clock40_m_hz[] = {
	/*
	 * unit_base_reg, unit_offset, mask, PEX data, wait_time,
	 * num_of_loops
	 */
	/* Bits[4:0]=0x3 - REF_FREF_SEL */
	{POWER_AND_PLL_CTRL_REG, 0x800, 0x1f, {0x3}, 0, 0},
	/* Bits[10]=0x1  - REFCLK_SEL */
	{MISC_REG, 0x800, 0x400, {0x400}, 0, 0},
	/* Bits[7:0]=0xc - CFG_PM_RXDLOZ_WAIT */
	{GLOBAL_PM_CTRL, 0x800, 0xff, {0xc}, 0, 0},
};

/* PEX - configuration seq for REF_CLOCK_100MHz */
struct op_params pex_config_ref_clock100_m_hz[] = {
	/*
	 * unit_base_reg, unit_offset, mask, PEX data, wait_time,
	 * num_of_loops
	 */
	/* Bits[4:0]=0x0  - REF_FREF_SEL */
	{POWER_AND_PLL_CTRL_REG, 0x800, 0x1f, {0x0}, 0, 0},
	/* Bit[10]=0x0    - REFCLK_SEL */
	{MISC_REG, 0x800, 0x400, {0x0}, 0, 0},
	/* Bits[7:0]=0x1e - CFG_PM_RXDLOZ_WAIT */
	{GLOBAL_PM_CTRL, 0x800, 0xff, {0x1e}, 0, 0},
};

/*
 *    USB2
 */

struct op_params usb2_power_up_params[] = {
	/*
	 * unit_base_reg, unit_offset, mask, USB2 data, wait_time,
	 * num_of_loops
	 */
	/* Init phy 0 */
	{0x18440, 0x0 /*NA*/, 0xffffffff, {0x62}, 0, 0},
	/* Init phy 1 */
	{0x18444, 0x0 /*NA*/, 0xffffffff, {0x62}, 0, 0},
	/* Init phy 2 */
	{0x18448, 0x0 /*NA*/, 0xffffffff, {0x62}, 0, 0},
	/* Phy offset 0x0 - PLL_CONTROL0  */
	{0xc0000, 0x0 /*NA*/, 0xffffffff, {0x40605205}, 0, 0},
	{0xc001c, 0x0 /*NA*/, 0xffffffff, {0x39f16ce}, 0, 0},
	{0xc201c, 0x0 /*NA*/, 0xffffffff, {0x39f16ce}, 0, 0},
	{0xc401c, 0x0 /*NA*/, 0xffffffff, {0x39f16ce}, 0, 0},
	/* Phy offset 0x1 - PLL_CONTROL1 */
	{0xc0004, 0x0 /*NA*/, 0x1, {0x1}, 0, 0},
	/* Phy0 register 3  - TX Channel control 0 */
	{0xc000c, 0x0 /*NA*/, 0x1000000, {0x1000000}, 0, 0},
	/* Phy0 register 3  - TX Channel control 0 */
	{0xc200c, 0x0 /*NA*/, 0x1000000, {0x1000000}, 0, 0},
	/* Phy0 register 3  - TX Channel control 0 */
	{0xc400c, 0x0 /*NA*/, 0x1000000, {0x1000000}, 0, 0},
	/* check PLLCAL_DONE is set and IMPCAL_DONE is set */
	{0xc0008, 0x0 /*NA*/, 0x80800000, {0x80800000}, 1, 1000},
	/* check REG_SQCAL_DONE  is set */
	{0xc0018, 0x0 /*NA*/, 0x80000000, {0x80000000}, 1, 1000},
	/* check PLL_READY  is set */
	{0xc0000, 0x0 /*NA*/, 0x80000000, {0x80000000}, 1, 1000}
};

/*
 *    QSGMII
 */

/* QSGMII - power up seq */
struct op_params qsgmii_port_power_up_params[] = {
	/*
	 * unit_base_reg, unit_offset, mask, QSGMII data, wait_time,
	 * num_of_loops
	 */
	/* Connect the QSGMII to Gigabit Ethernet units */
	{QSGMII_CONTROL_REG1, 0x0, 0x40000000, {0x40000000}, 0, 0},
	/* Power Up */
	{COMMON_PHY_CONFIGURATION1_REG, 0x28, 0xf0006, {0x80002}, 0, 0},
	/* Unreset */
	{COMMON_PHY_CONFIGURATION1_REG, 0x28, 0x7800, {0x6000}, 0, 0},
	/* Phy Selector */
	{POWER_AND_PLL_CTRL_REG, 0x800, 0xff, {0xfc81}, 0, 0},
	/* Ref clock source select */
	{MISC_REG, 0x800, 0x4c0, {0x480}, 0, 0}
};

/* QSGMII - speed config seq */
struct op_params qsgmii_port_speed_config_params[] = {
	/*
	 * unit_base_reg, unit_offset, mask, QSGMII data, wait_time,
	 * num_of_loops
	 */
	/* Baud Rate */
	{COMMON_PHY_CONFIGURATION1_REG, 0x28, 0x3fc00000, {0xcc00000}, 0, 0},
	/* Phy Gen RX and TX */
	{ISOLATE_REG, 0x800, 0xff, {0x33}, 0, 0},
	/* Bus Width */
	{LOOPBACK_REG, 0x800, 0xe, {0x2}, 0, 0}
};

/* QSGMII - Select electrical param seq */
struct op_params qsgmii_port_electrical_config_params[] = {
	/*
	 * unit_base_reg, unit_offset, mask, QSGMII data, wait_time,
	 * num_of_loops
	 */
	/* Slew rate and emphasis */
	{G1_SETTINGS_0_REG, 0x800, 0x8000, {0x0}, 0, 0}
};

/* QSGMII - TX config seq */
struct op_params qsgmii_port_tx_config_params1[] = {
	/*
	 * unit_base_reg, unit_offset, mask, QSGMII data, wait_time,
	 * num_of_loops
	 */
	{GLUE_REG, 0x800, 0x1800, {0x800}, 0, 0},
	/* Sft Reset pulse */
	{RESET_DFE_REG, 0x800, 0x401, {0x401}, 0, 0},
	/* Sft Reset pulse */
	{RESET_DFE_REG, 0x800, 0x401, {0x0}, 0, 0},
	/* Lane align */
	{LANE_ALIGN_REG0, 0x800, 0x1000, {0x1000}, 0, 0},
	/* Power up PLL, RX and TX */
	{COMMON_PHY_CONFIGURATION1_REG, 0x28, 0x70000, {0x70000}, 0, 0},
	/* Tx driver output idle */
	{COMMON_PHY_CONFIGURATION1_REG, 0x28, 0x80000, {0x80000}, 0, 0}
};

struct op_params qsgmii_port_tx_config_params2[] = {
	/*
	 * unit_base_reg, unit_offset, mask, QSGMII data, wait_time,
	 * num_of_loops
	 */
	/* Wait for PHY power up sequence to finish */
	{COMMON_PHY_STATUS1_REG, 0x28, 0xc, {0xc}, 10, 1000},
	/* Assert Rx Init and Tx driver output valid */
	{COMMON_PHY_CONFIGURATION1_REG, 0x28, 0x40080000, {0x40000000}, 0, 0},
	/* Wait for PHY power up sequence to finish */
	{COMMON_PHY_STATUS1_REG, 0x28, 0x1, {0x1}, 1, 1000},
	/* De-assert Rx Init */
	{COMMON_PHY_CONFIGURATION1_REG, 0x28, 0x40000000, {0x0}, 0, 0}
};

/* SERDES_POWER_DOWN */
struct op_params serdes_power_down_params[] = {
	{COMMON_PHY_CONFIGURATION1_REG, 0x28, (0xf << 11), {(0x3 << 11)},
	 0, 0},
	{COMMON_PHY_CONFIGURATION1_REG, 0x28, (0x7 << 16), {0}, 0, 0}
};

/*
 * hws_ctrl_serdes_rev_get
 *
 * DESCRIPTION: Get the Serdes revision number
 *
 * INPUT: config_field - Field description enum
 *
 * OUTPUT: None
 *
 * RETURN:
 *		8bit Serdes revision number
 */
u8 hws_ctrl_serdes_rev_get(void)
{
#ifdef CONFIG_ARMADA_38X
	/* for A38x-Z1 */
	if (sys_env_device_rev_get() == MV_88F68XX_Z1_ID)
		return MV_SERDES_REV_1_2;
#endif

	/* for A39x-Z1, A38x-A0 */
	return MV_SERDES_REV_2_1;
}

u32 hws_serdes_topology_verify(enum serdes_type serdes_type, u32 serdes_id,
			       enum serdes_mode serdes_mode)
{
	u32 test_result = 0;
	u8 serd_max_num, unit_numb;
	enum unit_id unit_id;

	if (serdes_type > RXAUI) {
		printf("%s: Warning: Wrong serdes type %s serdes#%d\n",
		       __func__, serdes_type_to_string[serdes_type], serdes_id);
		return MV_FAIL;
	}

	unit_id = serdes_type_to_unit_info[serdes_type].serdes_unit_id;
	unit_numb = serdes_type_to_unit_info[serdes_type].serdes_unit_num;
	serd_max_num = sys_env_unit_max_num_get(unit_id);

	/* if didn't exceed amount of required Serdes lanes for current type */
	if (serdes_lane_in_use_count[unit_id][unit_numb] != 0) {
		/* update amount of required Serdes lanes for current type */
		serdes_lane_in_use_count[unit_id][unit_numb]--;

		/*
		 * If reached the exact amount of required Serdes lanes for
		 * current type
		 */
		if (serdes_lane_in_use_count[unit_id][unit_numb] == 0) {
			if (((serdes_type <= PEX3)) &&
			    ((serdes_mode == PEX_END_POINT_X4) ||
			     (serdes_mode == PEX_ROOT_COMPLEX_X4))) {
				/* PCiex4 uses 2 SerDes */
				serdes_unit_count[PEX_UNIT_ID] += 2;
			} else {
				serdes_unit_count[unit_id]++;
			}

			/* test SoC unit count limitation */
			if (serdes_unit_count[unit_id] > serd_max_num) {
				test_result = WRONG_NUMBER_OF_UNITS;
			} else if (unit_numb >= serd_max_num) {
				/* test SoC unit number limitation */
				test_result = UNIT_NUMBER_VIOLATION;
			}
		}
	} else {
		test_result = SERDES_ALREADY_IN_USE;
	}

	if (test_result == SERDES_ALREADY_IN_USE) {
		printf("%s: Error: serdes lane %d is configured to type %s: type already in use\n",
		       __func__, serdes_id,
		       serdes_type_to_string[serdes_type]);
		return MV_FAIL;
	} else if (test_result == WRONG_NUMBER_OF_UNITS) {
		printf("%s: Warning: serdes lane %d is set to type %s.\n",
		       __func__, serdes_id,
		       serdes_type_to_string[serdes_type]);
		printf("%s: Maximum supported lanes are already set to this type (limit = %d)\n",
		       __func__, serd_max_num);
		return MV_FAIL;
	} else if (test_result == UNIT_NUMBER_VIOLATION) {
		printf("%s: Warning: serdes lane %d type is %s: current device support only %d units of this type.\n",
		       __func__, serdes_id,
		       serdes_type_to_string[serdes_type],
		       serd_max_num);
		return MV_FAIL;
	}

	return MV_OK;
}

void hws_serdes_xaui_topology_verify(void)
{
	/*
	 * If XAUI is in use - serdes_lane_in_use_count has to be = 0;
	 * if it is not in use hast be = 4
	 */
	if ((serdes_lane_in_use_count[XAUI_UNIT_ID][0] != 0) &&
	    (serdes_lane_in_use_count[XAUI_UNIT_ID][0] != 4)) {
		printf("%s: Warning: wrong number of lanes is set to XAUI - %d\n",
		       __func__, serdes_lane_in_use_count[XAUI_UNIT_ID][0]);
		printf("%s: XAUI has to be defined on 4 lanes\n", __func__);
	}

	/*
	 * If RXAUI is in use - serdes_lane_in_use_count has to be = 0;
	 * if it is not in use hast be = 2
	 */
	if ((serdes_lane_in_use_count[RXAUI_UNIT_ID][0] != 0) &&
	    (serdes_lane_in_use_count[RXAUI_UNIT_ID][0] != 2)) {
		printf("%s: Warning: wrong number of lanes is set to RXAUI - %d\n",
		       __func__, serdes_lane_in_use_count[RXAUI_UNIT_ID][0]);
		printf("%s: RXAUI has to be defined on 2 lanes\n", __func__);
	}
}

int hws_serdes_seq_db_init(void)
{
	u8 serdes_rev = hws_ctrl_serdes_rev_get();

	DEBUG_INIT_FULL_S("\n### serdes_seq38x_init ###\n");

	if (serdes_rev == MV_SERDES_REV_NA) {
		printf("hws_serdes_seq_db_init: serdes revision number is not supported\n");
		return MV_NOT_SUPPORTED;
	}

	/* SATA_PORT_0_ONLY_POWER_UP_SEQ sequence init */
	serdes_seq_db[SATA_PORT_0_ONLY_POWER_UP_SEQ].op_params_ptr =
	    sata_port0_power_up_params;
	serdes_seq_db[SATA_PORT_0_ONLY_POWER_UP_SEQ].cfg_seq_size =
	    sizeof(sata_port0_power_up_params) / sizeof(struct op_params);
	serdes_seq_db[SATA_PORT_0_ONLY_POWER_UP_SEQ].data_arr_idx = SATA;

	/* SATA_PORT_1_ONLY_POWER_UP_SEQ sequence init */
	serdes_seq_db[SATA_PORT_1_ONLY_POWER_UP_SEQ].op_params_ptr =
	    sata_port1_power_up_params;
	serdes_seq_db[SATA_PORT_1_ONLY_POWER_UP_SEQ].cfg_seq_size =
	    sizeof(sata_port1_power_up_params) / sizeof(struct op_params);
	serdes_seq_db[SATA_PORT_1_ONLY_POWER_UP_SEQ].data_arr_idx = SATA;

	/* SATA_POWER_UP_SEQ sequence init */
	serdes_seq_db[SATA_POWER_UP_SEQ].op_params_ptr =
	    sata_and_sgmii_power_up_params;
	serdes_seq_db[SATA_POWER_UP_SEQ].cfg_seq_size =
	    sizeof(sata_and_sgmii_power_up_params) / sizeof(struct op_params);
	serdes_seq_db[SATA_POWER_UP_SEQ].data_arr_idx = SATA;

	/* SATA_1_5_SPEED_CONFIG_SEQ sequence init */
	serdes_seq_db[SATA_1_5_SPEED_CONFIG_SEQ].op_params_ptr =
	    sata_and_sgmii_speed_config_params;
	serdes_seq_db[SATA_1_5_SPEED_CONFIG_SEQ].cfg_seq_size =
	    sizeof(sata_and_sgmii_speed_config_params) /
		sizeof(struct op_params);
	serdes_seq_db[SATA_1_5_SPEED_CONFIG_SEQ].data_arr_idx = SATA;

	/* SATA_3_SPEED_CONFIG_SEQ sequence init */
	serdes_seq_db[SATA_3_SPEED_CONFIG_SEQ].op_params_ptr =
	    sata_and_sgmii_speed_config_params;
	serdes_seq_db[SATA_3_SPEED_CONFIG_SEQ].cfg_seq_size =
	    sizeof(sata_and_sgmii_speed_config_params) /
		sizeof(struct op_params);
	serdes_seq_db[SATA_3_SPEED_CONFIG_SEQ].data_arr_idx = SATA;

	/* SATA_6_SPEED_CONFIG_SEQ sequence init */
	serdes_seq_db[SATA_6_SPEED_CONFIG_SEQ].op_params_ptr =
	    sata_and_sgmii_speed_config_params;
	serdes_seq_db[SATA_6_SPEED_CONFIG_SEQ].cfg_seq_size =
	    sizeof(sata_and_sgmii_speed_config_params) /
		sizeof(struct op_params);
	serdes_seq_db[SATA_6_SPEED_CONFIG_SEQ].data_arr_idx = SATA;

	/* SATA_ELECTRICAL_CONFIG_SEQ seq sequence init */
	if (serdes_rev == MV_SERDES_REV_1_2) {
		serdes_seq_db[SATA_ELECTRICAL_CONFIG_SEQ].op_params_ptr =
		    sata_electrical_config_serdes_rev1_params;
		serdes_seq_db[SATA_ELECTRICAL_CONFIG_SEQ].cfg_seq_size =
		    sizeof(sata_electrical_config_serdes_rev1_params) /
		    sizeof(struct op_params);
	} else {
		serdes_seq_db[SATA_ELECTRICAL_CONFIG_SEQ].op_params_ptr =
		    sata_electrical_config_serdes_rev2_params;
		serdes_seq_db[SATA_ELECTRICAL_CONFIG_SEQ].cfg_seq_size =
		    sizeof(sata_electrical_config_serdes_rev2_params) /
		    sizeof(struct op_params);
	}
	serdes_seq_db[SATA_ELECTRICAL_CONFIG_SEQ].data_arr_idx = SATA;

	/* SATA_TX_CONFIG_SEQ sequence init */
	serdes_seq_db[SATA_TX_CONFIG_SEQ1].op_params_ptr =
	    sata_and_sgmii_tx_config_params1;
	serdes_seq_db[SATA_TX_CONFIG_SEQ1].cfg_seq_size =
	    sizeof(sata_and_sgmii_tx_config_params1) / sizeof(struct op_params);
	serdes_seq_db[SATA_TX_CONFIG_SEQ1].data_arr_idx = SATA;

	/* SATA_PORT_0_ONLY_TX_CONFIG_SEQ sequence init */
	serdes_seq_db[SATA_PORT_0_ONLY_TX_CONFIG_SEQ].op_params_ptr =
	    sata_port0_tx_config_params;
	serdes_seq_db[SATA_PORT_0_ONLY_TX_CONFIG_SEQ].cfg_seq_size =
	    sizeof(sata_port0_tx_config_params) / sizeof(struct op_params);
	serdes_seq_db[SATA_PORT_0_ONLY_TX_CONFIG_SEQ].data_arr_idx = SATA;

	/* SATA_PORT_1_ONLY_TX_CONFIG_SEQ sequence init */
	serdes_seq_db[SATA_PORT_1_ONLY_TX_CONFIG_SEQ].op_params_ptr =
	    sata_port1_tx_config_params;
	serdes_seq_db[SATA_PORT_1_ONLY_TX_CONFIG_SEQ].cfg_seq_size =
	    sizeof(sata_port1_tx_config_params) / sizeof(struct op_params);
	serdes_seq_db[SATA_PORT_1_ONLY_TX_CONFIG_SEQ].data_arr_idx = SATA;

	/* SATA_TX_CONFIG_SEQ2 sequence init */
	if (serdes_rev == MV_SERDES_REV_1_2) {
		serdes_seq_db[SATA_TX_CONFIG_SEQ2].op_params_ptr =
		    sata_and_sgmii_tx_config_serdes_rev1_params2;
		serdes_seq_db[SATA_TX_CONFIG_SEQ2].cfg_seq_size =
		    sizeof(sata_and_sgmii_tx_config_serdes_rev1_params2) /
		    sizeof(struct op_params);
	} else {
		serdes_seq_db[SATA_TX_CONFIG_SEQ2].op_params_ptr =
		    sata_and_sgmii_tx_config_serdes_rev2_params2;
		serdes_seq_db[SATA_TX_CONFIG_SEQ2].cfg_seq_size =
		    sizeof(sata_and_sgmii_tx_config_serdes_rev2_params2) /
		    sizeof(struct op_params);
	}
	serdes_seq_db[SATA_TX_CONFIG_SEQ2].data_arr_idx = SATA;

	/* SGMII_POWER_UP_SEQ sequence init */
	serdes_seq_db[SGMII_POWER_UP_SEQ].op_params_ptr =
	    sata_and_sgmii_power_up_params;
	serdes_seq_db[SGMII_POWER_UP_SEQ].cfg_seq_size =
	    sizeof(sata_and_sgmii_power_up_params) / sizeof(struct op_params);
	serdes_seq_db[SGMII_POWER_UP_SEQ].data_arr_idx = SGMII;

	/* SGMII_1_25_SPEED_CONFIG_SEQ sequence init */
	serdes_seq_db[SGMII_1_25_SPEED_CONFIG_SEQ].op_params_ptr =
	    sata_and_sgmii_speed_config_params;
	serdes_seq_db[SGMII_1_25_SPEED_CONFIG_SEQ].cfg_seq_size =
	    sizeof(sata_and_sgmii_speed_config_params) /
		sizeof(struct op_params);
	serdes_seq_db[SGMII_1_25_SPEED_CONFIG_SEQ].data_arr_idx = SGMII;

	/* SGMII_3_125_SPEED_CONFIG_SEQ sequence init */
	serdes_seq_db[SGMII_3_125_SPEED_CONFIG_SEQ].op_params_ptr =
	    sata_and_sgmii_speed_config_params;
	serdes_seq_db[SGMII_3_125_SPEED_CONFIG_SEQ].cfg_seq_size =
	    sizeof(sata_and_sgmii_speed_config_params) /
		sizeof(struct op_params);
	serdes_seq_db[SGMII_3_125_SPEED_CONFIG_SEQ].data_arr_idx = SGMII_3_125;

	/* SGMII_ELECTRICAL_CONFIG_SEQ seq sequence init */
	if (serdes_rev == MV_SERDES_REV_1_2) {
		serdes_seq_db[SGMII_ELECTRICAL_CONFIG_SEQ].op_params_ptr =
		    sgmii_electrical_config_serdes_rev1_params;
		serdes_seq_db[SGMII_ELECTRICAL_CONFIG_SEQ].cfg_seq_size =
		    sizeof(sgmii_electrical_config_serdes_rev1_params) /
		    sizeof(struct op_params);
	} else {
		serdes_seq_db[SGMII_ELECTRICAL_CONFIG_SEQ].op_params_ptr =
		    sgmii_electrical_config_serdes_rev2_params;
		serdes_seq_db[SGMII_ELECTRICAL_CONFIG_SEQ].cfg_seq_size =
		    sizeof(sgmii_electrical_config_serdes_rev2_params) /
		    sizeof(struct op_params);
	}
	serdes_seq_db[SGMII_ELECTRICAL_CONFIG_SEQ].data_arr_idx = SGMII;

	/* SGMII_TX_CONFIG_SEQ sequence init */
	serdes_seq_db[SGMII_TX_CONFIG_SEQ1].op_params_ptr =
	    sata_and_sgmii_tx_config_params1;
	serdes_seq_db[SGMII_TX_CONFIG_SEQ1].cfg_seq_size =
	    sizeof(sata_and_sgmii_tx_config_params1) / sizeof(struct op_params);
	serdes_seq_db[SGMII_TX_CONFIG_SEQ1].data_arr_idx = SGMII;

	/* SGMII_TX_CONFIG_SEQ sequence init */
	if (serdes_rev == MV_SERDES_REV_1_2) {
		serdes_seq_db[SGMII_TX_CONFIG_SEQ2].op_params_ptr =
		    sata_and_sgmii_tx_config_serdes_rev1_params2;
		serdes_seq_db[SGMII_TX_CONFIG_SEQ2].cfg_seq_size =
		    sizeof(sata_and_sgmii_tx_config_serdes_rev1_params2) /
		    sizeof(struct op_params);
	} else {
		serdes_seq_db[SGMII_TX_CONFIG_SEQ2].op_params_ptr =
		    sata_and_sgmii_tx_config_serdes_rev2_params2;
		serdes_seq_db[SGMII_TX_CONFIG_SEQ2].cfg_seq_size =
		    sizeof(sata_and_sgmii_tx_config_serdes_rev2_params2) /
		    sizeof(struct op_params);
	}
	serdes_seq_db[SGMII_TX_CONFIG_SEQ2].data_arr_idx = SGMII;

	/* PEX_POWER_UP_SEQ sequence init */
	if (serdes_rev == MV_SERDES_REV_1_2) {
		serdes_seq_db[PEX_POWER_UP_SEQ].op_params_ptr =
		    pex_and_usb3_power_up_serdes_rev1_params;
		serdes_seq_db[PEX_POWER_UP_SEQ].cfg_seq_size =
		    sizeof(pex_and_usb3_power_up_serdes_rev1_params) /
		    sizeof(struct op_params);
	} else {
		serdes_seq_db[PEX_POWER_UP_SEQ].op_params_ptr =
		    pex_and_usb3_power_up_serdes_rev2_params;
		serdes_seq_db[PEX_POWER_UP_SEQ].cfg_seq_size =
		    sizeof(pex_and_usb3_power_up_serdes_rev2_params) /
		    sizeof(struct op_params);
	}
	serdes_seq_db[PEX_POWER_UP_SEQ].data_arr_idx = PEX;

	/* PEX_2_5_SPEED_CONFIG_SEQ sequence init */
	serdes_seq_db[PEX_2_5_SPEED_CONFIG_SEQ].op_params_ptr =
	    pex_and_usb3_speed_config_params;
	serdes_seq_db[PEX_2_5_SPEED_CONFIG_SEQ].cfg_seq_size =
	    sizeof(pex_and_usb3_speed_config_params) / sizeof(struct op_params);
	serdes_seq_db[PEX_2_5_SPEED_CONFIG_SEQ].data_arr_idx =
		PEXSERDES_SPEED_2_5_GBPS;

	/* PEX_5_SPEED_CONFIG_SEQ sequence init */
	serdes_seq_db[PEX_5_SPEED_CONFIG_SEQ].op_params_ptr =
	    pex_and_usb3_speed_config_params;
	serdes_seq_db[PEX_5_SPEED_CONFIG_SEQ].cfg_seq_size =
	    sizeof(pex_and_usb3_speed_config_params) / sizeof(struct op_params);
	serdes_seq_db[PEX_5_SPEED_CONFIG_SEQ].data_arr_idx =
		PEXSERDES_SPEED_5_GBPS;

	/* PEX_ELECTRICAL_CONFIG_SEQ seq sequence init */
	if (serdes_rev == MV_SERDES_REV_1_2) {
		serdes_seq_db[PEX_ELECTRICAL_CONFIG_SEQ].op_params_ptr =
		    pex_electrical_config_serdes_rev1_params;
		serdes_seq_db[PEX_ELECTRICAL_CONFIG_SEQ].cfg_seq_size =
		    sizeof(pex_electrical_config_serdes_rev1_params) /
		    sizeof(struct op_params);
	} else {
		serdes_seq_db[PEX_ELECTRICAL_CONFIG_SEQ].op_params_ptr =
		    pex_electrical_config_serdes_rev2_params;
		serdes_seq_db[PEX_ELECTRICAL_CONFIG_SEQ].cfg_seq_size =
		    sizeof(pex_electrical_config_serdes_rev2_params) /
		    sizeof(struct op_params);
	}
	serdes_seq_db[PEX_ELECTRICAL_CONFIG_SEQ].data_arr_idx = PEX;

	/* PEX_TX_CONFIG_SEQ1 sequence init */
	serdes_seq_db[PEX_TX_CONFIG_SEQ1].op_params_ptr =
	    pex_and_usb3_tx_config_params1;
	serdes_seq_db[PEX_TX_CONFIG_SEQ1].cfg_seq_size =
	    sizeof(pex_and_usb3_tx_config_params1) / sizeof(struct op_params);
	serdes_seq_db[PEX_TX_CONFIG_SEQ1].data_arr_idx = PEX;

	/* PEX_TX_CONFIG_SEQ2 sequence init */
	serdes_seq_db[PEX_TX_CONFIG_SEQ2].op_params_ptr =
	    pex_and_usb3_tx_config_params2;
	serdes_seq_db[PEX_TX_CONFIG_SEQ2].cfg_seq_size =
	    sizeof(pex_and_usb3_tx_config_params2) / sizeof(struct op_params);
	serdes_seq_db[PEX_TX_CONFIG_SEQ2].data_arr_idx = PEX;

	/* PEX_TX_CONFIG_SEQ3 sequence init */
	serdes_seq_db[PEX_TX_CONFIG_SEQ3].op_params_ptr =
	    pex_and_usb3_tx_config_params3;
	serdes_seq_db[PEX_TX_CONFIG_SEQ3].cfg_seq_size =
	    sizeof(pex_and_usb3_tx_config_params3) / sizeof(struct op_params);
	serdes_seq_db[PEX_TX_CONFIG_SEQ3].data_arr_idx = PEX;

	/* PEX_BY_4_CONFIG_SEQ sequence init */
	serdes_seq_db[PEX_BY_4_CONFIG_SEQ].op_params_ptr =
	    pex_by4_config_params;
	serdes_seq_db[PEX_BY_4_CONFIG_SEQ].cfg_seq_size =
	    sizeof(pex_by4_config_params) / sizeof(struct op_params);
	serdes_seq_db[PEX_BY_4_CONFIG_SEQ].data_arr_idx = PEX;

	/* PEX_CONFIG_REF_CLOCK_25MHZ_SEQ sequence init */
	serdes_seq_db[PEX_CONFIG_REF_CLOCK_25MHZ_SEQ].op_params_ptr =
	    pex_config_ref_clock25_m_hz;
	serdes_seq_db[PEX_CONFIG_REF_CLOCK_25MHZ_SEQ].cfg_seq_size =
	    sizeof(pex_config_ref_clock25_m_hz) / sizeof(struct op_params);
	serdes_seq_db[PEX_CONFIG_REF_CLOCK_25MHZ_SEQ].data_arr_idx = PEX;

	/* PEX_ELECTRICAL_CONFIG_REF_CLOCK_40MHZ_SEQ sequence init */
	serdes_seq_db[PEX_CONFIG_REF_CLOCK_40MHZ_SEQ].op_params_ptr =
	    pex_config_ref_clock40_m_hz;
	serdes_seq_db[PEX_CONFIG_REF_CLOCK_40MHZ_SEQ].cfg_seq_size =
	    sizeof(pex_config_ref_clock40_m_hz) / sizeof(struct op_params);
	serdes_seq_db[PEX_CONFIG_REF_CLOCK_40MHZ_SEQ].data_arr_idx = PEX;

	/* PEX_CONFIG_REF_CLOCK_100MHZ_SEQ sequence init */
	serdes_seq_db[PEX_CONFIG_REF_CLOCK_100MHZ_SEQ].op_params_ptr =
	    pex_config_ref_clock100_m_hz;
	serdes_seq_db[PEX_CONFIG_REF_CLOCK_100MHZ_SEQ].cfg_seq_size =
	    sizeof(pex_config_ref_clock100_m_hz) / sizeof(struct op_params);
	serdes_seq_db[PEX_CONFIG_REF_CLOCK_100MHZ_SEQ].data_arr_idx = PEX;

	/* USB3_POWER_UP_SEQ sequence init */
	if (serdes_rev == MV_SERDES_REV_1_2) {
		serdes_seq_db[USB3_POWER_UP_SEQ].op_params_ptr =
		    pex_and_usb3_power_up_serdes_rev1_params;
		serdes_seq_db[USB3_POWER_UP_SEQ].cfg_seq_size =
		    sizeof(pex_and_usb3_power_up_serdes_rev1_params) /
		    sizeof(struct op_params);
	} else {
		serdes_seq_db[USB3_POWER_UP_SEQ].op_params_ptr =
		    pex_and_usb3_power_up_serdes_rev2_params;
		serdes_seq_db[USB3_POWER_UP_SEQ].cfg_seq_size =
		    sizeof(pex_and_usb3_power_up_serdes_rev2_params) /
		    sizeof(struct op_params);
	}
	serdes_seq_db[USB3_POWER_UP_SEQ].data_arr_idx = USB3;

	/* USB3_HOST_SPEED_CONFIG_SEQ sequence init */
	serdes_seq_db[USB3_HOST_SPEED_CONFIG_SEQ].op_params_ptr =
	    pex_and_usb3_speed_config_params;
	serdes_seq_db[USB3_HOST_SPEED_CONFIG_SEQ].cfg_seq_size =
	    sizeof(pex_and_usb3_speed_config_params) / sizeof(struct op_params);
	serdes_seq_db[USB3_HOST_SPEED_CONFIG_SEQ].data_arr_idx =
	    USB3SERDES_SPEED_5_GBPS_HOST;

	/* USB3_DEVICE_SPEED_CONFIG_SEQ sequence init */
	serdes_seq_db[USB3_DEVICE_SPEED_CONFIG_SEQ].op_params_ptr =
	    pex_and_usb3_speed_config_params;
	serdes_seq_db[USB3_DEVICE_SPEED_CONFIG_SEQ].cfg_seq_size =
	    sizeof(pex_and_usb3_speed_config_params) / sizeof(struct op_params);
	serdes_seq_db[USB3_DEVICE_SPEED_CONFIG_SEQ].data_arr_idx =
	    USB3SERDES_SPEED_5_GBPS_DEVICE;

	/* USB3_ELECTRICAL_CONFIG_SEQ seq sequence init */
	if (serdes_rev == MV_SERDES_REV_1_2) {
		serdes_seq_db[USB3_ELECTRICAL_CONFIG_SEQ].op_params_ptr =
		    usb3_electrical_config_serdes_rev1_params;
		serdes_seq_db[USB3_ELECTRICAL_CONFIG_SEQ].cfg_seq_size =
		    sizeof(usb3_electrical_config_serdes_rev1_params) /
		    sizeof(struct op_params);
	} else {
		serdes_seq_db[USB3_ELECTRICAL_CONFIG_SEQ].op_params_ptr =
		    usb3_electrical_config_serdes_rev2_params;
		serdes_seq_db[USB3_ELECTRICAL_CONFIG_SEQ].cfg_seq_size =
		    sizeof(usb3_electrical_config_serdes_rev2_params) /
		    sizeof(struct op_params);
	}
	serdes_seq_db[USB3_ELECTRICAL_CONFIG_SEQ].data_arr_idx = USB3;

	/* USB3_TX_CONFIG_SEQ sequence init */
	serdes_seq_db[USB3_TX_CONFIG_SEQ1].op_params_ptr =
	    pex_and_usb3_tx_config_params1;
	serdes_seq_db[USB3_TX_CONFIG_SEQ1].cfg_seq_size =
	    sizeof(pex_and_usb3_tx_config_params1) / sizeof(struct op_params);
	serdes_seq_db[USB3_TX_CONFIG_SEQ1].data_arr_idx = USB3;

	/* USB3_TX_CONFIG_SEQ sequence init */
	serdes_seq_db[USB3_TX_CONFIG_SEQ2].op_params_ptr =
	    pex_and_usb3_tx_config_params2;
	serdes_seq_db[USB3_TX_CONFIG_SEQ2].cfg_seq_size =
	    sizeof(pex_and_usb3_tx_config_params2) / sizeof(struct op_params);
	serdes_seq_db[USB3_TX_CONFIG_SEQ2].data_arr_idx = USB3;

	/* USB3_TX_CONFIG_SEQ sequence init */
	serdes_seq_db[USB3_TX_CONFIG_SEQ3].op_params_ptr =
	    pex_and_usb3_tx_config_params3;
	serdes_seq_db[USB3_TX_CONFIG_SEQ3].cfg_seq_size =
	    sizeof(pex_and_usb3_tx_config_params3) / sizeof(struct op_params);
	serdes_seq_db[USB3_TX_CONFIG_SEQ3].data_arr_idx = USB3;

	/* USB2_POWER_UP_SEQ sequence init */
	serdes_seq_db[USB2_POWER_UP_SEQ].op_params_ptr = usb2_power_up_params;
	serdes_seq_db[USB2_POWER_UP_SEQ].cfg_seq_size =
	    sizeof(usb2_power_up_params) / sizeof(struct op_params);
	serdes_seq_db[USB2_POWER_UP_SEQ].data_arr_idx = 0;

	/* USB3_DEVICE_CONFIG_SEQ sequence init */
	serdes_seq_db[USB3_DEVICE_CONFIG_SEQ].op_params_ptr =
	    usb3_device_config_params;
	serdes_seq_db[USB3_DEVICE_CONFIG_SEQ].cfg_seq_size =
	    sizeof(usb3_device_config_params) / sizeof(struct op_params);
	serdes_seq_db[USB3_DEVICE_CONFIG_SEQ].data_arr_idx = 0;	/* Not relevant */

	/* SERDES_POWER_DOWN_SEQ sequence init */
	serdes_seq_db[SERDES_POWER_DOWN_SEQ].op_params_ptr =
	    serdes_power_down_params;
	serdes_seq_db[SERDES_POWER_DOWN_SEQ].cfg_seq_size =
	    sizeof(serdes_power_down_params) /
		sizeof(struct op_params);
	serdes_seq_db[SERDES_POWER_DOWN_SEQ].data_arr_idx = FIRST_CELL;

	if (serdes_rev == MV_SERDES_REV_2_1) {
		/* QSGMII_POWER_UP_SEQ sequence init */
		serdes_seq_db[QSGMII_POWER_UP_SEQ].op_params_ptr =
		    qsgmii_port_power_up_params;
		serdes_seq_db[QSGMII_POWER_UP_SEQ].cfg_seq_size =
		    sizeof(qsgmii_port_power_up_params) /
			sizeof(struct op_params);
		serdes_seq_db[QSGMII_POWER_UP_SEQ].data_arr_idx =
		    QSGMII_SEQ_IDX;

		/* QSGMII_5_SPEED_CONFIG_SEQ sequence init */
		serdes_seq_db[QSGMII_5_SPEED_CONFIG_SEQ].op_params_ptr =
		    qsgmii_port_speed_config_params;
		serdes_seq_db[QSGMII_5_SPEED_CONFIG_SEQ].cfg_seq_size =
		    sizeof(qsgmii_port_speed_config_params) /
			sizeof(struct op_params);
		serdes_seq_db[QSGMII_5_SPEED_CONFIG_SEQ].data_arr_idx =
		    QSGMII_SEQ_IDX;

		/* QSGMII_ELECTRICAL_CONFIG_SEQ seq sequence init */
		serdes_seq_db[QSGMII_ELECTRICAL_CONFIG_SEQ].op_params_ptr =
		    qsgmii_port_electrical_config_params;
		serdes_seq_db[QSGMII_ELECTRICAL_CONFIG_SEQ].cfg_seq_size =
		    sizeof(qsgmii_port_electrical_config_params) /
		    sizeof(struct op_params);
		serdes_seq_db[QSGMII_ELECTRICAL_CONFIG_SEQ].data_arr_idx =
		    QSGMII_SEQ_IDX;

		/* QSGMII_TX_CONFIG_SEQ sequence init */
		serdes_seq_db[QSGMII_TX_CONFIG_SEQ1].op_params_ptr =
		    qsgmii_port_tx_config_params1;
		serdes_seq_db[QSGMII_TX_CONFIG_SEQ1].cfg_seq_size =
		    sizeof(qsgmii_port_tx_config_params1) /
			sizeof(struct op_params);
		serdes_seq_db[QSGMII_TX_CONFIG_SEQ1].data_arr_idx =
		    QSGMII_SEQ_IDX;

		/* QSGMII_TX_CONFIG_SEQ sequence init */
		serdes_seq_db[QSGMII_TX_CONFIG_SEQ2].op_params_ptr =
		    qsgmii_port_tx_config_params2;
		serdes_seq_db[QSGMII_TX_CONFIG_SEQ2].cfg_seq_size =
		    sizeof(qsgmii_port_tx_config_params2) /
			sizeof(struct op_params);
		serdes_seq_db[QSGMII_TX_CONFIG_SEQ2].data_arr_idx =
		    QSGMII_SEQ_IDX;
	}

	return MV_OK;
}

enum serdes_seq serdes_type_and_speed_to_speed_seq(enum serdes_type serdes_type,
					      enum serdes_speed baud_rate)
{
	enum serdes_seq seq_id = SERDES_LAST_SEQ;

	DEBUG_INIT_FULL_S("\n### serdes_type_and_speed_to_speed_seq ###\n");
	switch (serdes_type) {
	case PEX0:
	case PEX1:
	case PEX2:
	case PEX3:
		if (baud_rate == SERDES_SPEED_2_5_GBPS)
			seq_id = PEX_2_5_SPEED_CONFIG_SEQ;
		else if (baud_rate == SERDES_SPEED_5_GBPS)
			seq_id = PEX_5_SPEED_CONFIG_SEQ;
		break;
	case USB3_HOST0:
	case USB3_HOST1:
		if (baud_rate == SERDES_SPEED_5_GBPS)
			seq_id = USB3_HOST_SPEED_CONFIG_SEQ;
		break;
	case USB3_DEVICE:
		if (baud_rate == SERDES_SPEED_5_GBPS)
			seq_id = USB3_DEVICE_SPEED_CONFIG_SEQ;
		break;
	case SATA0:
	case SATA1:
	case SATA2:
	case SATA3:
		if (baud_rate == SERDES_SPEED_1_5_GBPS)
			seq_id = SATA_1_5_SPEED_CONFIG_SEQ;
		else if (baud_rate == SERDES_SPEED_3_GBPS)
			seq_id = SATA_3_SPEED_CONFIG_SEQ;
		else if (baud_rate == SERDES_SPEED_6_GBPS)
			seq_id = SATA_6_SPEED_CONFIG_SEQ;
		break;
	case SGMII0:
	case SGMII1:
	case SGMII2:
#ifdef CONFIG_ARMADA_39X
	case SGMII3:
#endif
		if (baud_rate == SERDES_SPEED_1_25_GBPS)
			seq_id = SGMII_1_25_SPEED_CONFIG_SEQ;
		else if (baud_rate == SERDES_SPEED_3_125_GBPS)
			seq_id = SGMII_3_125_SPEED_CONFIG_SEQ;
		break;
	case QSGMII:
		seq_id = QSGMII_5_SPEED_CONFIG_SEQ;
		break;
#ifdef CONFIG_ARMADA_39X
	case XAUI:
		seq_id = XAUI_3_125_SPEED_CONFIG_SEQ;
		break;
	case RXAUI:
		seq_id = RXAUI_6_25_SPEED_CONFIG_SEQ;
		break;
#endif
	default:
		return SERDES_LAST_SEQ;
	}

	return seq_id;
}

static void print_topology_details(const struct serdes_map *serdes_map,
								u8 count)
{
	u32 lane_num;

	DEBUG_INIT_S("board SerDes lanes topology details:\n");

	DEBUG_INIT_S(" | Lane #  | Speed |  Type       |\n");
	DEBUG_INIT_S(" --------------------------------\n");
	for (lane_num = 0; lane_num < count; lane_num++) {
		if (serdes_map[lane_num].serdes_type == DEFAULT_SERDES)
			continue;
		DEBUG_INIT_S(" |   ");
		DEBUG_INIT_D(hws_get_physical_serdes_num(lane_num), 1);
		DEBUG_INIT_S("    |  ");
		DEBUG_INIT_D(serdes_map[lane_num].serdes_speed, 2);
		DEBUG_INIT_S("   |  ");
		DEBUG_INIT_S((char *)
			     serdes_type_to_string[serdes_map[lane_num].
						   serdes_type]);
		DEBUG_INIT_S("\t|\n");
	}
	DEBUG_INIT_S(" --------------------------------\n");
}

int hws_pre_serdes_init_config(void)
{
	u32 data;

	/*
	 * Configure Core PLL
	 */
	/*
	 * set PLL parameters
	 * bits[2:0]  =0x3 (Core-PLL Kdiv)
	 * bits[20:12]=0x9f (Core-PLL Ndiv)
	 * bits[24:21]=0x7(Core-PLL VCO Band)
	 * bits[28:25]=0x1(Core-PLL Rlf)
	 * bits[31:29]=0x2(Core-PLL charge-pump adjust)
	 */
	reg_write(CORE_PLL_PARAMETERS_REG, 0x42e9f003);

	/* Enable PLL Configuration */
	data = reg_read(CORE_PLL_CONFIG_REG);
	data = SET_BIT(data, 9);
	reg_write(CORE_PLL_CONFIG_REG, data);

	return MV_OK;
}

int serdes_phy_config(void)
{
	struct serdes_map *serdes_map;
	u8 serdes_count;

	DEBUG_INIT_FULL_S("\n### ctrl_high_speed_serdes_phy_config ###\n");

	DEBUG_INIT_S("High speed PHY - Version: ");
	DEBUG_INIT_S(SERDES_VERSION);
	DEBUG_INIT_S("\n");

	/* Init serdes sequences DB */
	if (hws_serdes_seq_init() != MV_OK) {
		printf("hws_ctrl_high_speed_serdes_phy_config: Error: Serdes initialization fail\n");
		return MV_FAIL;
	}

	/* Board topology load */
	DEBUG_INIT_FULL_S
	    ("ctrl_high_speed_serdes_phy_config: Loading board topology..\n");
	CHECK_STATUS(hws_board_topology_load(&serdes_map, &serdes_count));
	if (serdes_count > hws_serdes_get_max_lane()) {
		printf("Error: too many serdes lanes specified by board\n");
		return MV_FAIL;
	}

	/* print topology */
	print_topology_details(serdes_map, serdes_count);
	CHECK_STATUS(hws_pre_serdes_init_config());

	/* Power-Up sequence */
	DEBUG_INIT_FULL_S
		("ctrl_high_speed_serdes_phy_config: Starting serdes power up sequence\n");

	CHECK_STATUS(hws_power_up_serdes_lanes(serdes_map, serdes_count));

	DEBUG_INIT_FULL_S
		("\n### ctrl_high_speed_serdes_phy_config ended successfully ###\n");

	DEBUG_INIT_S(ENDED_OK);

	return MV_OK;
}

int serdes_polarity_config(u32 serdes_num, int is_rx)
{
	u32 data;
	u32 reg_addr;
	u8 bit_off = (is_rx) ? 11 : 10;

	reg_addr = SERDES_REGS_LANE_BASE_OFFSET(serdes_num) + SYNC_PATTERN_REG;
	data = reg_read(reg_addr);
	data = SET_BIT(data, bit_off);
	reg_write(reg_addr, data);

	return MV_OK;
}

int hws_power_up_serdes_lanes(struct serdes_map *serdes_map, u8 count)
{
	u32 serdes_id, serdes_lane_num;
	enum ref_clock ref_clock;
	enum serdes_type serdes_type;
	enum serdes_speed serdes_speed;
	enum serdes_mode serdes_mode;
	int serdes_rx_polarity_swap;
	int serdes_tx_polarity_swap;
	int is_pex_enabled = 0;

	/*
	 * is_pex_enabled:
	 * Flag which indicates that one of the Serdes is of PEX.
	 * In this case, PEX unit will be initialized after Serdes power-up
	 */

	DEBUG_INIT_FULL_S("\n### hws_power_up_serdes_lanes ###\n");

	/* COMMON PHYS SELECTORS register configuration */
	DEBUG_INIT_FULL_S
	    ("hws_power_up_serdes_lanes: Updating COMMON PHYS SELECTORS reg\n");
	CHECK_STATUS(hws_update_serdes_phy_selectors(serdes_map, count));

	/* per Serdes Power Up */
	for (serdes_id = 0; serdes_id < count; serdes_id++) {
		DEBUG_INIT_FULL_S
		    ("calling serdes_power_up_ctrl: serdes lane number ");
		DEBUG_INIT_FULL_D_10(serdes_lane_num, 1);
		DEBUG_INIT_FULL_S("\n");

		serdes_lane_num = hws_get_physical_serdes_num(serdes_id);
		serdes_type = serdes_map[serdes_id].serdes_type;
		serdes_speed = serdes_map[serdes_id].serdes_speed;
		serdes_mode = serdes_map[serdes_id].serdes_mode;
		serdes_rx_polarity_swap = serdes_map[serdes_id].swap_rx;
		serdes_tx_polarity_swap = serdes_map[serdes_id].swap_tx;

		/* serdes lane is not in use */
		if (serdes_type == DEFAULT_SERDES)
			continue;
		else if (serdes_type <= PEX3)	/* PEX type */
			is_pex_enabled = 1;

		ref_clock = hws_serdes_get_ref_clock_val(serdes_type);
		if (ref_clock == REF_CLOCK_UNSUPPORTED) {
			DEBUG_INIT_S
			    ("hws_power_up_serdes_lanes: unsupported ref clock\n");
			return MV_NOT_SUPPORTED;
		}
		CHECK_STATUS(serdes_power_up_ctrl(serdes_lane_num,
						  1,
						  serdes_type,
						  serdes_speed,
						  serdes_mode, ref_clock));

		/* RX Polarity config */
		if (serdes_rx_polarity_swap)
			CHECK_STATUS(serdes_polarity_config
				     (serdes_lane_num, 1));

		/* TX Polarity config */
		if (serdes_tx_polarity_swap)
			CHECK_STATUS(serdes_polarity_config
				     (serdes_lane_num, 0));
	}

	if (is_pex_enabled) {
		/* Set PEX_TX_CONFIG_SEQ sequence for PEXx4 mode.
		   After finish the Power_up sequence for all lanes,
		   the lanes should be released from reset state.       */
		CHECK_STATUS(hws_pex_tx_config_seq(serdes_map, count));

		/* PEX configuration */
		CHECK_STATUS(hws_pex_config(serdes_map, count));
	}

	/* USB2 configuration */
	DEBUG_INIT_FULL_S("hws_power_up_serdes_lanes: init USB2 Phys\n");
	CHECK_STATUS(mv_seq_exec(0 /* not relevant */ , USB2_POWER_UP_SEQ));

	DEBUG_INIT_FULL_S
	    ("### hws_power_up_serdes_lanes ended successfully ###\n");

	return MV_OK;
}

int ctrl_high_speed_serdes_phy_config(void)
{
	return hws_ctrl_high_speed_serdes_phy_config();
}

static int serdes_pex_usb3_pipe_delay_w_a(u32 serdes_num, u8 serdes_type)
{
	u32 reg_data;

	/* WA for A380 Z1 relevant for lanes 3,4,5 only */
	if (serdes_num >= 3) {
		reg_data = reg_read(GENERAL_PURPOSE_RESERVED0_REG);
		/* set delay on pipe -
		 * When lane 3 is connected to a MAC of Pex -> set bit 7 to 1.
		 * When lane 3 is connected to a MAC of USB3 -> set bit 7 to 0.
		 * When lane 4 is connected to a MAC of Pex -> set bit 8 to 1.
		 * When lane 4 is connected to a MAC of USB3 -> set bit 8 to 0.
		 * When lane 5 is connected to a MAC of Pex -> set bit 8 to 1.
		 * When lane 5 is connected to a MAC of USB3 -> set bit 8 to 0.
		 */
		if (serdes_type == PEX)
			reg_data |= 1 << (7 + (serdes_num - 3));
		if (serdes_type == USB3) {
			/* USB3 */
			reg_data &= ~(1 << (7 + (serdes_num - 3)));
		}
		reg_write(GENERAL_PURPOSE_RESERVED0_REG, reg_data);
	}

	return MV_OK;
}

/*
 * hws_serdes_pex_ref_clock_satr_get -
 *
 * DESCRIPTION: Get the reference clock value from DEVICE_SAMPLE_AT_RESET1_REG
 *              and check:
 *              bit[2] for PEX#0, bit[3] for PEX#1, bit[30] for PEX#2, bit[31]
 *              for PEX#3.
 *              If bit=0 --> REF_CLOCK_100MHz
 *              If bit=1 && DEVICE_SAMPLE_AT_RESET2_REG bit[0]=0
 *              --> REF_CLOCK_25MHz
 *              If bit=1 && DEVICE_SAMPLE_AT_RESET2_REG bit[0]=1
 *              --> REF_CLOCK_40MHz
 *
 * INPUT:        serdes_type - Type of Serdes
 *
 * OUTPUT:       pex_satr   -  Return the REF_CLOCK value:
 *                            REF_CLOCK_25MHz, REF_CLOCK_40MHz or REF_CLOCK_100MHz
 *
 * RETURNS:      MV_OK        - for success
 *               MV_BAD_PARAM - for fail
 */
int hws_serdes_pex_ref_clock_satr_get(enum serdes_type serdes_type, u32 *pex_satr)
{
	u32 data, reg_satr1;

	reg_satr1 = reg_read(DEVICE_SAMPLE_AT_RESET1_REG);

	switch (serdes_type) {
	case PEX0:
		data = REF_CLK_SELECTOR_VAL_PEX0(reg_satr1);
		break;
	case PEX1:
		data = REF_CLK_SELECTOR_VAL_PEX1(reg_satr1);
		break;
	case PEX2:
		data = REF_CLK_SELECTOR_VAL_PEX2(reg_satr1);
		break;
	case PEX3:
		data = REF_CLK_SELECTOR_VAL_PEX3(reg_satr1);
		break;
	default:
		printf("%s: Error: SerDes type %d is not supported\n",
		       __func__, serdes_type);
		return MV_BAD_PARAM;
	}

	*pex_satr = data;

	return MV_OK;
}

u32 hws_serdes_get_ref_clock_val(enum serdes_type serdes_type)
{
	u32 pex_satr;
	enum ref_clock ref_clock;

	DEBUG_INIT_FULL_S("\n### hws_serdes_get_ref_clock_val ###\n");

	if (serdes_type >= LAST_SERDES_TYPE)
		return REF_CLOCK_UNSUPPORTED;

	/* read ref clock from S@R */
	ref_clock = hws_serdes_silicon_ref_clock_get();

	if (serdes_type > PEX3) {
		/* for all Serdes types but PCIe */
		return ref_clock;
	}

	/* for PCIe, need also to check PCIe S@R */
	CHECK_STATUS(hws_serdes_pex_ref_clock_satr_get
		     (serdes_type, &pex_satr));

	if (pex_satr == 0) {
		return REF_CLOCK_100MHZ;
	} else if (pex_satr == 1) {
		/* value of 1 means we can use ref clock from SoC (as other Serdes types) */
		return ref_clock;
	} else {
		printf
		    ("%s: Error: REF_CLK_SELECTOR_VAL for SerDes type %d is wrong\n",
		     __func__, serdes_type);
		return REF_CLOCK_UNSUPPORTED;
	}
}

int serdes_power_up_ctrl(u32 serdes_num, int serdes_power_up,
			 enum serdes_type serdes_type,
			 enum serdes_speed baud_rate,
			 enum serdes_mode serdes_mode, enum ref_clock ref_clock)
{
	u32 sata_idx, pex_idx, sata_port;
	enum serdes_seq speed_seq_id;
	u32 reg_data;
	int is_pex_by1;

	DEBUG_INIT_FULL_S("\n### serdes_power_up_ctrl ###\n");

	if (serdes_power_up == 1) {	/* Serdes power up */
		DEBUG_INIT_FULL_S
		    ("serdes_power_up_ctrl: executing power up.. ");
		DEBUG_INIT_FULL_C("serdes num = ", serdes_num, 2);
		DEBUG_INIT_FULL_C("serdes type = ", serdes_type, 2);

		DEBUG_INIT_FULL_S("Going access 1");

		/* Getting the Speed Select sequence id */
		speed_seq_id =
			serdes_type_and_speed_to_speed_seq(serdes_type,
							   baud_rate);
		if (speed_seq_id == SERDES_LAST_SEQ) {
			printf
			    ("serdes_power_up_ctrl: serdes type %d and speed %d are not supported together\n",
			     serdes_type, baud_rate);

			return MV_BAD_PARAM;
		}

		/* Executing power up, ref clock set, speed config and TX config */
		switch (serdes_type) {
		case PEX0:
		case PEX1:
		case PEX2:
		case PEX3:
			if (hws_ctrl_serdes_rev_get() == MV_SERDES_REV_1_2) {
				CHECK_STATUS(serdes_pex_usb3_pipe_delay_w_a
					     (serdes_num, PEX));
			}

			is_pex_by1 = (serdes_mode == PEX_ROOT_COMPLEX_X1) ||
				(serdes_mode == PEX_END_POINT_X1);
			pex_idx = serdes_type - PEX0;

			if ((is_pex_by1 == 1) || (serdes_type == PEX0)) {
				/* For PEX by 4, init only the PEX 0 */
				reg_data = reg_read(SOC_CONTROL_REG1);
				if (is_pex_by1 == 1)
					reg_data |= 0x4000;
				else
					reg_data &= ~0x4000;
				reg_write(SOC_CONTROL_REG1, reg_data);

				reg_data =
				    reg_read(((PEX_IF_REGS_BASE(pex_idx)) +
					      0x6c));
				reg_data &= ~0x3f0;
				if (is_pex_by1 == 1)
					reg_data |= 0x10;
				else
					reg_data |= 0x40;
				reg_write(((PEX_IF_REGS_BASE(pex_idx)) + 0x6c),
					  reg_data);

				reg_data =
				    reg_read(((PEX_IF_REGS_BASE(pex_idx)) +
					      0x6c));
				reg_data &= ~0xf;
				reg_data |= 0x2;
				reg_write(((PEX_IF_REGS_BASE(pex_idx)) + 0x6c),
					  reg_data);

				reg_data =
				    reg_read(((PEX_IF_REGS_BASE(pex_idx)) +
					      0x70));
				reg_data &= ~0x40;
				reg_data |= 0x40;
				reg_write(((PEX_IF_REGS_BASE(pex_idx)) + 0x70),
					  reg_data);
			}

			CHECK_STATUS(mv_seq_exec(serdes_num, PEX_POWER_UP_SEQ));
			if (is_pex_by1 == 0) {
				/*
				 * for PEX by 4 - use the PEX index as the
				 * seq array index
				 */
				serdes_seq_db[PEX_BY_4_CONFIG_SEQ].
				    data_arr_idx = pex_idx;
				CHECK_STATUS(mv_seq_exec
					     (serdes_num, PEX_BY_4_CONFIG_SEQ));
			}

			CHECK_STATUS(hws_ref_clock_set
				     (serdes_num, serdes_type, ref_clock));
			CHECK_STATUS(mv_seq_exec(serdes_num, speed_seq_id));
			CHECK_STATUS(mv_seq_exec
				     (serdes_num, PEX_ELECTRICAL_CONFIG_SEQ));

			if (is_pex_by1 == 1) {
				CHECK_STATUS(mv_seq_exec
					     (serdes_num, PEX_TX_CONFIG_SEQ2));
				CHECK_STATUS(mv_seq_exec
					     (serdes_num, PEX_TX_CONFIG_SEQ3));
				CHECK_STATUS(mv_seq_exec
					     (serdes_num, PEX_TX_CONFIG_SEQ1));
			}
			udelay(20);

			break;
		case USB3_HOST0:
		case USB3_HOST1:
		case USB3_DEVICE:
			if (hws_ctrl_serdes_rev_get() == MV_SERDES_REV_1_2) {
				CHECK_STATUS(serdes_pex_usb3_pipe_delay_w_a
					     (serdes_num, USB3));
			}
			CHECK_STATUS(mv_seq_exec
				     (serdes_num, USB3_POWER_UP_SEQ));
			CHECK_STATUS(hws_ref_clock_set
				     (serdes_num, serdes_type, ref_clock));
			CHECK_STATUS(mv_seq_exec(serdes_num, speed_seq_id));
			if (serdes_type == USB3_DEVICE) {
				CHECK_STATUS(mv_seq_exec
					     (serdes_num,
					      USB3_DEVICE_CONFIG_SEQ));
			}
			CHECK_STATUS(mv_seq_exec
				     (serdes_num, USB3_ELECTRICAL_CONFIG_SEQ));
			CHECK_STATUS(mv_seq_exec
				     (serdes_num, USB3_TX_CONFIG_SEQ1));
			CHECK_STATUS(mv_seq_exec
				     (serdes_num, USB3_TX_CONFIG_SEQ2));
			CHECK_STATUS(mv_seq_exec
				     (serdes_num, USB3_TX_CONFIG_SEQ3));

			udelay(10000);
			break;
		case SATA0:
		case SATA1:
		case SATA2:
		case SATA3:
			sata_idx = ((serdes_type == SATA0) ||
				    (serdes_type == SATA1)) ? 0 : 1;
			sata_port = ((serdes_type == SATA0) ||
				     (serdes_type == SATA2)) ? 0 : 1;

			CHECK_STATUS(mv_seq_exec
				     (sata_idx, (sata_port == 0) ?
				      SATA_PORT_0_ONLY_POWER_UP_SEQ :
				      SATA_PORT_1_ONLY_POWER_UP_SEQ));
			CHECK_STATUS(mv_seq_exec
				     (serdes_num, SATA_POWER_UP_SEQ));
			CHECK_STATUS(hws_ref_clock_set
				     (serdes_num, serdes_type, ref_clock));
			CHECK_STATUS(mv_seq_exec(serdes_num, speed_seq_id));
			CHECK_STATUS(mv_seq_exec
				     (serdes_num, SATA_ELECTRICAL_CONFIG_SEQ));
			CHECK_STATUS(mv_seq_exec
				     (serdes_num, SATA_TX_CONFIG_SEQ1));
			CHECK_STATUS(mv_seq_exec
				     (sata_idx, (sata_port == 0) ?
				      SATA_PORT_0_ONLY_TX_CONFIG_SEQ :
				      SATA_PORT_1_ONLY_TX_CONFIG_SEQ));
			CHECK_STATUS(mv_seq_exec
				     (serdes_num, SATA_TX_CONFIG_SEQ2));

			udelay(10000);
			break;
		case SGMII0:
		case SGMII1:
		case SGMII2:
			CHECK_STATUS(mv_seq_exec
				     (serdes_num, SGMII_POWER_UP_SEQ));
			CHECK_STATUS(hws_ref_clock_set
				     (serdes_num, serdes_type, ref_clock));
			CHECK_STATUS(mv_seq_exec(serdes_num, speed_seq_id));
			CHECK_STATUS(mv_seq_exec
				     (serdes_num, SGMII_ELECTRICAL_CONFIG_SEQ));
			CHECK_STATUS(mv_seq_exec
				     (serdes_num, SGMII_TX_CONFIG_SEQ1));
			CHECK_STATUS(mv_seq_exec
				     (serdes_num, SGMII_TX_CONFIG_SEQ2));

			/* GBE configuration */
			reg_data = reg_read(GBE_CONFIGURATION_REG);
			/* write the SGMII index */
			reg_data |= 0x1 << (serdes_type - SGMII0);
			reg_write(GBE_CONFIGURATION_REG, reg_data);

			break;
		case QSGMII:
			if (hws_ctrl_serdes_rev_get() < MV_SERDES_REV_2_1)
				return MV_NOT_SUPPORTED;

			CHECK_STATUS(mv_seq_exec
				     (serdes_num, QSGMII_POWER_UP_SEQ));
			CHECK_STATUS(hws_ref_clock_set
				     (serdes_num, serdes_type, ref_clock));
			CHECK_STATUS(mv_seq_exec(serdes_num, speed_seq_id));
			CHECK_STATUS(mv_seq_exec
				     (serdes_num,
				      QSGMII_ELECTRICAL_CONFIG_SEQ));
			CHECK_STATUS(mv_seq_exec
				     (serdes_num, QSGMII_TX_CONFIG_SEQ1));
			CHECK_STATUS(mv_seq_exec
				     (serdes_num, QSGMII_TX_CONFIG_SEQ2));
			break;
		case SGMII3:
		case XAUI:
		case RXAUI:
			CHECK_STATUS(serdes_power_up_ctrl_ext
				     (serdes_num, serdes_power_up, serdes_type,
				      baud_rate, serdes_mode, ref_clock));
			break;
		default:
			DEBUG_INIT_S
			    ("serdes_power_up_ctrl: bad serdes_type parameter\n");
			return MV_BAD_PARAM;
		}
	} else {		/* Serdes power down */
		DEBUG_INIT_FULL_S("serdes_power_up: executing power down.. ");
		DEBUG_INIT_FULL_C("serdes num = ", serdes_num, 1);

		CHECK_STATUS(mv_seq_exec(serdes_num, SERDES_POWER_DOWN_SEQ));
	}

	DEBUG_INIT_FULL_C(
		"serdes_power_up_ctrl ended successfully for serdes ",
		serdes_num, 2);

	return MV_OK;
}

int hws_update_serdes_phy_selectors(struct serdes_map *serdes_map, u8 count)
{
	u32 lane_data, idx, serdes_lane_hw_num, reg_data = 0;
	enum serdes_type serdes_type;
	enum serdes_mode serdes_mode;
	u8 select_bit_off;
	int is_pex_x4 = 0;
	int updated_topology_print = 0;

	DEBUG_INIT_FULL_S("\n### hws_update_serdes_phy_selectors ###\n");
	DEBUG_INIT_FULL_S
	    ("Updating the COMMON PHYS SELECTORS register with the serdes types\n");

	if (hws_ctrl_serdes_rev_get() == MV_SERDES_REV_1_2)
		select_bit_off = 3;
	else
		select_bit_off = 4;

	/*
	 * Updating bits 0-17 in the COMMON PHYS SELECTORS register
	 * according to the serdes types
	 */
	for (idx = 0; idx < count; idx++) {
		serdes_type = serdes_map[idx].serdes_type;
		serdes_mode = serdes_map[idx].serdes_mode;
		serdes_lane_hw_num = hws_get_physical_serdes_num(idx);

		lane_data =
		    hws_serdes_get_phy_selector_val(serdes_lane_hw_num,
						    serdes_type);

		if (serdes_type == DEFAULT_SERDES)
			continue;

		if (hws_serdes_topology_verify
		    (serdes_type, idx, serdes_mode) != MV_OK) {
			serdes_map[idx].serdes_type =
			    DEFAULT_SERDES;
			printf("%s: SerDes lane #%d is  disabled\n", __func__,
			       serdes_lane_hw_num);
			updated_topology_print = 1;
			continue;
		}

		/*
		 * Checking if the board topology configuration includes
		 * PEXx4 - for the next step
		 */
		if ((serdes_mode == PEX_END_POINT_X4) ||
		    (serdes_mode == PEX_ROOT_COMPLEX_X4)) {
			/* update lane data to the 3 next SERDES lanes */
			lane_data =
			    common_phys_selectors_pex_by4_lanes
			    [serdes_lane_hw_num];
			if (serdes_type == PEX0)
				is_pex_x4 = 1;
		}

		if (lane_data == NA) {
			printf
			    ("%s: Warning: SerDes lane #%d and type %d are not supported together\n",
			     __func__, serdes_lane_hw_num, serdes_mode);
			serdes_map[idx].serdes_type = DEFAULT_SERDES;
			printf("%s: SerDes lane #%d is  disabled\n", __func__,
			       serdes_lane_hw_num);
			continue;
		}

		/*
		 * Updating the data that will be written to
		 * COMMON_PHYS_SELECTORS_REG
		 */
		reg_data |= (lane_data <<
			     (select_bit_off * serdes_lane_hw_num));
	}

	/*
	 * Check that number of used lanes for XAUI and RXAUI
	 * (if used) is right
	 */
	hws_serdes_xaui_topology_verify();

	/* Print topology */
	if (updated_topology_print)
		print_topology_details(serdes_map, count);

	/*
	 * Updating the PEXx4 Enable bit in the COMMON PHYS SELECTORS
	 * register for PEXx4 mode
	 */
	reg_data |= (is_pex_x4 == 1) ? (0x1 << PEX_X4_ENABLE_OFFS) : 0;

	/* Updating the COMMON PHYS SELECTORS register */
	reg_write(COMMON_PHYS_SELECTORS_REG, reg_data);

	return MV_OK;
}

int hws_ref_clock_set(u32 serdes_num, enum serdes_type serdes_type,
		      enum ref_clock ref_clock)
{
	u32 data1 = 0, data2 = 0, data3 = 0, reg_data;

	DEBUG_INIT_FULL_S("\n### hws_ref_clock_set ###\n");

	if (hws_is_serdes_active(serdes_num) != 1) {
		printf("%s: SerDes lane #%d is not Active\n", __func__,
		       serdes_num);
		return MV_BAD_PARAM;
	}

	switch (serdes_type) {
	case PEX0:
	case PEX1:
	case PEX2:
	case PEX3:
		switch (ref_clock) {
		case REF_CLOCK_25MHZ:
			CHECK_STATUS(mv_seq_exec
				     (serdes_num,
				      PEX_CONFIG_REF_CLOCK_25MHZ_SEQ));
			return MV_OK;
		case REF_CLOCK_100MHZ:
			CHECK_STATUS(mv_seq_exec
				     (serdes_num,
				      PEX_CONFIG_REF_CLOCK_100MHZ_SEQ));
			return MV_OK;
#ifdef CONFIG_ARMADA_39X
		case REF_CLOCK_40MHZ:
			CHECK_STATUS(mv_seq_exec
				     (serdes_num,
				      PEX_CONFIG_REF_CLOCK_40MHZ_SEQ));
			return MV_OK;
#endif
		default:
			printf
			    ("%s: Error: ref_clock %d for SerDes lane #%d, type %d is not supported\n",
			     __func__, ref_clock, serdes_num, serdes_type);
			return MV_BAD_PARAM;
		}
	case USB3_HOST0:
	case USB3_HOST1:
	case USB3_DEVICE:
		if (ref_clock == REF_CLOCK_25MHZ) {
			data1 = POWER_AND_PLL_CTRL_REG_25MHZ_VAL_2;
			data2 = GLOBAL_PM_CTRL_REG_25MHZ_VAL;
			data3 = LANE_CFG4_REG_25MHZ_VAL;
		} else if (ref_clock == REF_CLOCK_40MHZ) {
			data1 = POWER_AND_PLL_CTRL_REG_40MHZ_VAL;
			data2 = GLOBAL_PM_CTRL_REG_40MHZ_VAL;
			data3 = LANE_CFG4_REG_40MHZ_VAL;
		} else {
			printf
			    ("hws_ref_clock_set: ref clock is not valid for serdes type %d\n",
			     serdes_type);
			return MV_BAD_PARAM;
		}
		break;
	case SATA0:
	case SATA1:
	case SATA2:
	case SATA3:
	case SGMII0:
	case SGMII1:
	case SGMII2:
	case QSGMII:
		if (ref_clock == REF_CLOCK_25MHZ) {
			data1 = POWER_AND_PLL_CTRL_REG_25MHZ_VAL_1;
		} else if (ref_clock == REF_CLOCK_40MHZ) {
			data1 = POWER_AND_PLL_CTRL_REG_40MHZ_VAL;
		} else {
			printf
			    ("hws_ref_clock_set: ref clock is not valid for serdes type %d\n",
			     serdes_type);
			return MV_BAD_PARAM;
		}
		break;
#ifdef CONFIG_ARMADA_39X
	case SGMII3:
	case XAUI:
	case RXAUI:
		if (ref_clock == REF_CLOCK_25MHZ) {
			data1 = POWER_AND_PLL_CTRL_REG_25MHZ_VAL_1;
		} else if (ref_clock == REF_CLOCK_40MHZ) {
			data1 = POWER_AND_PLL_CTRL_REG_40MHZ_VAL;
		} else {
			printf
			    ("hws_ref_clock_set: ref clock is not valid for serdes type %d\n",
			     serdes_type);
			return MV_BAD_PARAM;
		}
		break;
#endif
	default:
		DEBUG_INIT_S("hws_ref_clock_set: not supported serdes type\n");
		return MV_BAD_PARAM;
	}

	/*
	 * Write the ref_clock to relevant SELECT_REF_CLOCK_REG bits and
	 * offset
	 */
	reg_data = reg_read(POWER_AND_PLL_CTRL_REG +
			    SERDES_REGS_LANE_BASE_OFFSET(serdes_num));
	reg_data &= POWER_AND_PLL_CTRL_REG_MASK;
	reg_data |= data1;
	reg_write(POWER_AND_PLL_CTRL_REG +
		  SERDES_REGS_LANE_BASE_OFFSET(serdes_num), reg_data);

	if ((serdes_type == USB3_HOST0) || (serdes_type == USB3_HOST1) ||
	    (serdes_type == USB3_DEVICE)) {
		reg_data = reg_read(GLOBAL_PM_CTRL +
				    SERDES_REGS_LANE_BASE_OFFSET(serdes_num));
		reg_data &= GLOBAL_PM_CTRL_REG_MASK;
		reg_data |= data2;
		reg_write(GLOBAL_PM_CTRL +
			  SERDES_REGS_LANE_BASE_OFFSET(serdes_num), reg_data);

		reg_data = reg_read(LANE_CFG4_REG +
				    SERDES_REGS_LANE_BASE_OFFSET(serdes_num));
		reg_data &= LANE_CFG4_REG_MASK;
		reg_data |= data3;
		reg_write(LANE_CFG4_REG +
			  SERDES_REGS_LANE_BASE_OFFSET(serdes_num), reg_data);
	}

	return MV_OK;
}

/*
 * hws_pex_tx_config_seq -
 *
 * DESCRIPTION:          Set PEX_TX_CONFIG_SEQ sequence init for PEXx4 mode
 * INPUT:                serdes_map       - The board topology map
 * OUTPUT:               None
 * RETURNS:              MV_OK           - for success
 *                       MV_BAD_PARAM    - for fail
 */
int hws_pex_tx_config_seq(const struct serdes_map *serdes_map, u8 count)
{
	enum serdes_mode serdes_mode;
	u32 serdes_lane_id, serdes_lane_hw_num;

	DEBUG_INIT_FULL_S("\n### hws_pex_tx_config_seq ###\n");

	/*
	 * For PEXx4: the pex_and_usb3_tx_config_params1/2/3
	 * configurations should run by setting each sequence for
	 * all 4 lanes.
	 */

	/* relese pipe soft reset for all lanes */
	for (serdes_lane_id = 0; serdes_lane_id < count; serdes_lane_id++) {
		serdes_mode = serdes_map[serdes_lane_id].serdes_mode;
		serdes_lane_hw_num =
		    hws_get_physical_serdes_num(serdes_lane_id);

		if ((serdes_mode == PEX_ROOT_COMPLEX_X4) ||
		    (serdes_mode == PEX_END_POINT_X4)) {
			CHECK_STATUS(mv_seq_exec
				     (serdes_lane_hw_num, PEX_TX_CONFIG_SEQ1));
		}
	}

	/* set phy soft reset for all lanes */
	for (serdes_lane_id = 0; serdes_lane_id < count; serdes_lane_id++) {
		serdes_mode = serdes_map[serdes_lane_id].serdes_mode;
		serdes_lane_hw_num =
		    hws_get_physical_serdes_num(serdes_lane_id);
		if ((serdes_mode == PEX_ROOT_COMPLEX_X4) ||
		    (serdes_mode == PEX_END_POINT_X4)) {
			CHECK_STATUS(mv_seq_exec
				     (serdes_lane_hw_num, PEX_TX_CONFIG_SEQ2));
		}
	}

	/* set phy soft reset for all lanes */
	for (serdes_lane_id = 0; serdes_lane_id < count; serdes_lane_id++) {
		serdes_mode = serdes_map[serdes_lane_id].serdes_mode;
		serdes_lane_hw_num =
		    hws_get_physical_serdes_num(serdes_lane_id);
		if ((serdes_mode == PEX_ROOT_COMPLEX_X4) ||
		    (serdes_mode == PEX_END_POINT_X4)) {
			CHECK_STATUS(mv_seq_exec
				     (serdes_lane_hw_num, PEX_TX_CONFIG_SEQ3));
		}
	}

	return MV_OK;
}
