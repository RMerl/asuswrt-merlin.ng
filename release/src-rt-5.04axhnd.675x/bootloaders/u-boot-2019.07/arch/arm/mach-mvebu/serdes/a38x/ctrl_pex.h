/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 */

#ifndef _CTRL_PEX_H
#define _CTRL_PEX_H

#include "high_speed_env_spec.h"

/* Sample at Reset */
#define MPP_SAMPLE_AT_RESET(id)		(0xe4200 + (id * 4))

/* PCI Express Control and Status Registers */
#define MAX_PEX_BUSSES			256

#define MISC_REGS_OFFSET		0x18200
#define MV_MISC_REGS_BASE		MISC_REGS_OFFSET
#define SOC_CTRL_REG			(MV_MISC_REGS_BASE + 0x4)

#define PEX_IF_REGS_OFFSET(if)		((if) > 0 ?			\
					 (0x40000 + ((if) - 1) * 0x4000) : \
					 0x80000)
#define PEX_IF_REGS_BASE(if)		(PEX_IF_REGS_OFFSET(if))
#define PEX_CAPABILITIES_REG(if)	((PEX_IF_REGS_BASE(if)) + 0x60)
#define PEX_LINK_CTRL_STATUS2_REG(if)	((PEX_IF_REGS_BASE(if)) + 0x90)
#define PEX_CTRL_REG(if)		((PEX_IF_REGS_BASE(if)) + 0x1a00)
#define PEX_STATUS_REG(if)		((PEX_IF_REGS_BASE(if)) + 0x1a04)
#define PEX_DBG_STATUS_REG(if)		((PEX_IF_REGS_BASE(if)) + 0x1a64)
#define PEX_LINK_CAPABILITY_REG		0x6c
#define PEX_LINK_CTRL_STAT_REG		0x70
#define PXSR_PEX_DEV_NUM_OFFS		16  /* Device Number Indication */
#define PXSR_PEX_DEV_NUM_MASK		(0x1f << PXSR_PEX_DEV_NUM_OFFS)
#define PXSR_PEX_BUS_NUM_OFFS		8 /* Bus Number Indication */
#define PXSR_PEX_BUS_NUM_MASK		(0xff << PXSR_PEX_BUS_NUM_OFFS)

/* PEX_CAPABILITIES_REG fields */
#define PCIE0_ENABLE_OFFS		0
#define PCIE0_ENABLE_MASK		(0x1 << PCIE0_ENABLE_OFFS)
#define PCIE1_ENABLE_OFFS		1
#define PCIE1_ENABLE_MASK		(0x1 << PCIE1_ENABLE_OFFS)
#define PCIE2_ENABLE_OFFS		2
#define PCIE2_ENABLE_MASK		(0x1 << PCIE2_ENABLE_OFFS)
#define PCIE3_ENABLE_OFFS		3
#define PCIE4_ENABLE_MASK		(0x1 << PCIE3_ENABLE_OFFS)

/* Controller revision info */
#define PEX_DEVICE_AND_VENDOR_ID	0x000
#define PEX_CFG_DIRECT_ACCESS(if, reg)	(PEX_IF_REGS_BASE(if) + (reg))

/* PCI Express Configuration Address Register */
#define PXCAR_REG_NUM_OFFS		2
#define PXCAR_REG_NUM_MAX		0x3f
#define PXCAR_REG_NUM_MASK		(PXCAR_REG_NUM_MAX << \
					 PXCAR_REG_NUM_OFFS)
#define PXCAR_FUNC_NUM_OFFS		8
#define PXCAR_FUNC_NUM_MAX		0x7
#define PXCAR_FUNC_NUM_MASK		(PXCAR_FUNC_NUM_MAX << \
					 PXCAR_FUNC_NUM_OFFS)
#define PXCAR_DEVICE_NUM_OFFS		11
#define PXCAR_DEVICE_NUM_MAX		0x1f
#define PXCAR_DEVICE_NUM_MASK		(PXCAR_DEVICE_NUM_MAX << \
					 PXCAR_DEVICE_NUM_OFFS)
#define PXCAR_BUS_NUM_OFFS		16
#define PXCAR_BUS_NUM_MAX		0xff
#define PXCAR_BUS_NUM_MASK		(PXCAR_BUS_NUM_MAX << \
					 PXCAR_BUS_NUM_OFFS)
#define PXCAR_EXT_REG_NUM_OFFS		24
#define PXCAR_EXT_REG_NUM_MAX		0xf

#define PEX_CFG_ADDR_REG(if)		((PEX_IF_REGS_BASE(if)) + 0x18f8)
#define PEX_CFG_DATA_REG(if)		((PEX_IF_REGS_BASE(if)) + 0x18fc)

#define PXCAR_REAL_EXT_REG_NUM_OFFS	8
#define PXCAR_REAL_EXT_REG_NUM_MASK	(0xf << PXCAR_REAL_EXT_REG_NUM_OFFS)

#define PXCAR_CONFIG_EN			BIT(31)
#define PEX_STATUS_AND_COMMAND		0x004
#define PXSAC_MABORT			BIT(29) /* Recieved Master Abort */

int hws_pex_config(const struct serdes_map *serdes_map, u8 count);
int pex_local_bus_num_set(u32 pex_if, u32 bus_num);
int pex_local_dev_num_set(u32 pex_if, u32 dev_num);
u32 pex_config_read(u32 pex_if, u32 bus, u32 dev, u32 func, u32 reg_off);

void board_pex_config(void);

#endif
