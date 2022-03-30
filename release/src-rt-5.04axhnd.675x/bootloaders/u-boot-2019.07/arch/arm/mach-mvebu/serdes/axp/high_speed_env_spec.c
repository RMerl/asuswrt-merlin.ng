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

MV_SERDES_CHANGE_M_PHY serdes_change_m_phy[] = {
	/* SERDES TYPE, Low REG OFFS, Low REG VALUE, Hi REG OFS, Hi REG VALUE */
	{
		/* PEX: Change of Slew Rate port0   */
		SERDES_UNIT_PEX, 0x0,
		(0x0F << 16) | 0x2a21, 0x0, (0x0F << 16) | 0x2a21
	}, {
		/* PEX: Change PLL BW port0                   */
		SERDES_UNIT_PEX, 0x0,
		(0x4F << 16) | 0x6219, 0x0, (0x4F << 16) | 0x6219
	}, {
		/* SATA: Slew rate change port 0  */
		SERDES_UNIT_SATA, 0x0083C, 0x8a31, 0x0083C, 0x8a31
	}, {
		/* SATA: Slew rate change port 0  */
		SERDES_UNIT_SATA, 0x00834, 0xc928, 0x00834, 0xc928
	}, {
		/* SATA: Slew rate change port 0  */
		SERDES_UNIT_SATA, 0x00838, 0x30f0, 0x00838, 0x30f0
	}, {
		/* SATA: Slew rate change port 0  */
		SERDES_UNIT_SATA, 0x00840, 0x30f5, 0x00840, 0x30f5
	}, {
		/* SGMII: FFE setting Port0         */
		SERDES_UNIT_SGMII0, 0x00E18, 0x989F, 0x00E18, 0x989F
	}, {
		/* SGMII: SELMUP and SELMUF Port0   */
		SERDES_UNIT_SGMII0, 0x00E38, 0x10FA, 0x00E38, 0x10FA
	}, {
		/* SGMII: Amplitude new setting gen2 Port3 */
		SERDES_UNIT_SGMII0, 0x00E34, 0xC968, 0x00E34, 0xC66C
	}, {
		/* QSGMII: Amplitude and slew rate change  */
		SERDES_UNIT_QSGMII, 0x72E34, 0xaa58, 0x72E34, 0xaa58
	}, {
		/* QSGMII: SELMUP and SELMUF               */
		SERDES_UNIT_QSGMII, 0x72e38, 0x10aF, 0x72e38, 0x10aF
	}, {
		/* QSGMII: 0x72e18                         */
		SERDES_UNIT_QSGMII, 0x72e18, 0x98AC, 0x72e18, 0x98AC
	}, {
		/* Null terminated */
		SERDES_UNIT_UNCONNECTED, 0, 0
	}
};

MV_BIN_SERDES_CFG db88f78xx0_serdes_cfg[] = {
	/* Z1B */
	{MV_PEX_ROOT_COMPLEX, 0x32221111, 0x11111111,
	 {PEX_BUS_MODE_X1, PEX_BUS_DISABLED, PEX_BUS_MODE_X4, PEX_BUS_MODE_X4},
	 0x0030, serdes_change_m_phy},			/* Default */
	{MV_PEX_ROOT_COMPLEX, 0x31211111, 0x11111111,
	 {PEX_BUS_MODE_X1, PEX_BUS_MODE_X1, PEX_BUS_MODE_X4, PEX_BUS_MODE_X4},
	 0x0030, serdes_change_m_phy},			/* PEX module */
	/* Z1A */
	{MV_PEX_ROOT_COMPLEX, 0x32220000, 0x00000000,
	 {PEX_BUS_DISABLED, PEX_BUS_DISABLED, PEX_BUS_DISABLED,
	  PEX_BUS_DISABLED}, 0x0030, serdes_change_m_phy}, /* Default - Z1A */
	{MV_PEX_ROOT_COMPLEX, 0x31210000, 0x00000000,
	 {PEX_BUS_DISABLED, PEX_BUS_MODE_X1, PEX_BUS_DISABLED, PEX_BUS_DISABLED},
	 0x0030, serdes_change_m_phy}	/* PEX module - Z1A */
};

MV_BIN_SERDES_CFG db88f78xx0rev2_serdes_cfg[] = {
	/* A0 */
	{MV_PEX_ROOT_COMPLEX, 0x33221111, 0x11111111,
	 {PEX_BUS_MODE_X1, PEX_BUS_DISABLED, PEX_BUS_MODE_X4, PEX_BUS_MODE_X4},
	 0x0030, serdes_change_m_phy}, /* Default: No Pex module, PEX0 x1, disabled */
	{MV_PEX_ROOT_COMPLEX, 0x33211111, 0x11111111,
	 {PEX_BUS_MODE_X1, PEX_BUS_MODE_X1, PEX_BUS_MODE_X4, PEX_BUS_MODE_X4},
	 0x0030, serdes_change_m_phy}, /* Pex module, PEX0 x1, PEX1 x1 */
	{MV_PEX_ROOT_COMPLEX, 0x33221111, 0x11111111,
	 {PEX_BUS_MODE_X4, PEX_BUS_DISABLED, PEX_BUS_MODE_X4, PEX_BUS_MODE_X4},
	 0x0030, serdes_change_m_phy}, /* no Pex module, PEX0 x4, PEX1 disabled */
	{MV_PEX_ROOT_COMPLEX, 0x33211111, 0x11111111,
	 {PEX_BUS_MODE_X4, PEX_BUS_MODE_X1, PEX_BUS_MODE_X4, PEX_BUS_MODE_X4},
	 0x0030, serdes_change_m_phy}, /* Pex module, PEX0 x4, PEX1 x1 */
	{MV_PEX_ROOT_COMPLEX, 0x11111111, 0x11111111,
	 {PEX_BUS_MODE_X1, PEX_BUS_MODE_X4, PEX_BUS_MODE_X4, PEX_BUS_MODE_X4},
	 0x0030, serdes_change_m_phy}, /* Pex module, PEX0 x1, PEX1 x4 */
	{MV_PEX_ROOT_COMPLEX, 0x11111111, 0x11111111,
	 {PEX_BUS_MODE_X4, PEX_BUS_MODE_X4, PEX_BUS_MODE_X4, PEX_BUS_MODE_X4},
	 0x0030, serdes_change_m_phy}, /* Pex module, PEX0 x4, PEX1 x4 */
};

MV_BIN_SERDES_CFG rd78460nas_serdes_cfg[] = {
	{MV_PEX_ROOT_COMPLEX, 0x00223001, 0x11111111,
	 {PEX_BUS_MODE_X1, PEX_BUS_DISABLED, PEX_BUS_MODE_X4, PEX_BUS_MODE_X4},
	 0x0030, serdes_change_m_phy}, /* Default */
	{MV_PEX_ROOT_COMPLEX, 0x33320201, 0x11111111,
	 {PEX_BUS_MODE_X1, PEX_BUS_DISABLED, PEX_BUS_MODE_X4, PEX_BUS_MODE_X4},
	 0x00f4, serdes_change_m_phy}, /* Switch module */
};

MV_BIN_SERDES_CFG rd78460_serdes_cfg[] = {
	{MV_PEX_ROOT_COMPLEX, 0x22321111, 0x00000000,
	 {PEX_BUS_MODE_X4, PEX_BUS_DISABLED, PEX_BUS_DISABLED, PEX_BUS_DISABLED},
	 0x0010, serdes_change_m_phy}, /* CPU0 */
	{MV_PEX_ROOT_COMPLEX, 0x00321111, 0x00000000,
	 {PEX_BUS_MODE_X4, PEX_BUS_DISABLED, PEX_BUS_DISABLED, PEX_BUS_DISABLED},
	 0x0010, serdes_change_m_phy} /* CPU1-3 */
};

MV_BIN_SERDES_CFG rd78460server_rev2_serdes_cfg[] = {
	{MV_PEX_ROOT_COMPLEX, 0x00321111, 0x00000000,
	 {PEX_BUS_MODE_X4, PEX_BUS_DISABLED, PEX_BUS_DISABLED, PEX_BUS_DISABLED},
	 0x0010, serdes_change_m_phy}, /* CPU0 */
	{MV_PEX_ROOT_COMPLEX, 0x00321111, 0x00000000,
	 {PEX_BUS_MODE_X4, PEX_BUS_DISABLED, PEX_BUS_DISABLED, PEX_BUS_DISABLED},
	 0x0010, serdes_change_m_phy} /* CPU1-3 */
};

MV_BIN_SERDES_CFG db78X60pcac_serdes_cfg[] = {
	{MV_PEX_END_POINT, 0x22321111, 0x00000000,
	 {PEX_BUS_MODE_X4, PEX_BUS_DISABLED, PEX_BUS_DISABLED, PEX_BUS_DISABLED},
	 0x0010, serdes_change_m_phy}	/* Default */
};

MV_BIN_SERDES_CFG db78X60pcacrev2_serdes_cfg[] = {
	{MV_PEX_END_POINT, 0x23321111, 0x00000000,
	 {PEX_BUS_MODE_X4, PEX_BUS_DISABLED, PEX_BUS_DISABLED, PEX_BUS_DISABLED},
	 0x0010, serdes_change_m_phy}	/* Default */
};

MV_BIN_SERDES_CFG fpga88f78xx0_serdes_cfg[] = {
	{MV_PEX_ROOT_COMPLEX, 0x00000000, 0x00000000,
	 {PEX_BUS_DISABLED, PEX_BUS_DISABLED, PEX_BUS_DISABLED, PEX_BUS_DISABLED},
	 0x0000, serdes_change_m_phy}	/* No PEX in FPGA */
};

MV_BIN_SERDES_CFG db78X60amc_serdes_cfg[] = {
	{MV_PEX_ROOT_COMPLEX, 0x33111111, 0x00010001,
	 {PEX_BUS_MODE_X4, PEX_BUS_MODE_X1, PEX_BUS_MODE_X1, PEX_BUS_MODE_X1},
	 0x0030, serdes_change_m_phy}	/* Default */
};

/*
 * ARMADA-XP CUSTOMER BOARD
 */
MV_BIN_SERDES_CFG rd78460customer_serdes_cfg[] = {
	{MV_PEX_ROOT_COMPLEX, 0x00223001, 0x11111111,
	 {PEX_BUS_MODE_X1, PEX_BUS_DISABLED, PEX_BUS_MODE_X4, PEX_BUS_MODE_X4},
	 0x00000030, serdes_change_m_phy}, /* Default */
	{MV_PEX_ROOT_COMPLEX, 0x33320201, 0x11111111,
	 {PEX_BUS_MODE_X1, PEX_BUS_DISABLED, PEX_BUS_MODE_X4, PEX_BUS_MODE_X4},
	 0x00000030, serdes_change_m_phy}, /* Switch module */
};

MV_BIN_SERDES_CFG rd78460AXP_GP_serdes_cfg[] = {
	{MV_PEX_ROOT_COMPLEX, 0x00223001, 0x11111111,
	 {PEX_BUS_MODE_X1, PEX_BUS_DISABLED, PEX_BUS_MODE_X4, PEX_BUS_MODE_X4},
	 0x0030, serdes_change_m_phy}	/* Default */
};

MV_BIN_SERDES_CFG *serdes_info_tbl[] = {
	db88f78xx0_serdes_cfg,
	rd78460_serdes_cfg,
	db78X60pcac_serdes_cfg,
	fpga88f78xx0_serdes_cfg,
	db88f78xx0rev2_serdes_cfg,
	rd78460nas_serdes_cfg,
	db78X60amc_serdes_cfg,
	db78X60pcacrev2_serdes_cfg,
	rd78460server_rev2_serdes_cfg,
	rd78460AXP_GP_serdes_cfg,
	rd78460customer_serdes_cfg
};

u8 rd78460gp_twsi_dev[] = { 0x4C, 0x4D, 0x4E };
u8 db88f78xx0rev2_twsi_dev[] = { 0x4C, 0x4D, 0x4E, 0x4F };
