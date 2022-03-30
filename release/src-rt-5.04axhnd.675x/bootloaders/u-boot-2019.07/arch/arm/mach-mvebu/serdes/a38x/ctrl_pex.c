// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 */

#include <common.h>
#include <spl.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>

#include "ctrl_pex.h"
#include "sys_env_lib.h"

__weak void board_pex_config(void)
{
	/* nothing in this weak default implementation */
}

int hws_pex_config(const struct serdes_map *serdes_map, u8 count)
{
	u32 pex_idx, tmp, next_busno, first_busno, temp_pex_reg,
	    temp_reg, addr, dev_id, ctrl_mode;
	enum serdes_type serdes_type;
	u32 idx;

	DEBUG_INIT_FULL_S("\n### hws_pex_config ###\n");

	for (idx = 0; idx < count; idx++) {
		serdes_type = serdes_map[idx].serdes_type;
		/* configuration for PEX only */
		if ((serdes_type != PEX0) && (serdes_type != PEX1) &&
		    (serdes_type != PEX2) && (serdes_type != PEX3))
			continue;

		if ((serdes_type != PEX0) &&
		    ((serdes_map[idx].serdes_mode == PEX_ROOT_COMPLEX_X4) ||
		     (serdes_map[idx].serdes_mode == PEX_END_POINT_X4))) {
			/* for PEX by4 - relevant for the first port only */
			continue;
		}

		pex_idx = serdes_type - PEX0;
		tmp = reg_read(PEX_CAPABILITIES_REG(pex_idx));
		tmp &= ~(0xf << 20);
		tmp |= (0x4 << 20);
		reg_write(PEX_CAPABILITIES_REG(pex_idx), tmp);
	}

	tmp = reg_read(SOC_CTRL_REG);
	tmp &= ~0x03;

	for (idx = 0; idx < count; idx++) {
		serdes_type = serdes_map[idx].serdes_type;
		if ((serdes_type != PEX0) &&
		    ((serdes_map[idx].serdes_mode == PEX_ROOT_COMPLEX_X4) ||
		     (serdes_map[idx].serdes_mode == PEX_END_POINT_X4))) {
			/* for PEX by4 - relevant for the first port only */
			continue;
		}

		switch (serdes_type) {
		case PEX0:
			tmp |= 0x1 << PCIE0_ENABLE_OFFS;
			break;
		case PEX1:
			tmp |= 0x1 << PCIE1_ENABLE_OFFS;
			break;
		case PEX2:
			tmp |= 0x1 << PCIE2_ENABLE_OFFS;
			break;
		case PEX3:
			tmp |= 0x1 << PCIE3_ENABLE_OFFS;
			break;
		default:
			break;
		}
	}

	reg_write(SOC_CTRL_REG, tmp);

	/* Support gen1/gen2 */
	DEBUG_INIT_FULL_S("Support gen1/gen2\n");

	board_pex_config();

	next_busno = 0;
	mdelay(150);

	for (idx = 0; idx < count; idx++) {
		serdes_type = serdes_map[idx].serdes_type;
		DEBUG_INIT_FULL_S(" serdes_type=0x");
		DEBUG_INIT_FULL_D(serdes_type, 8);
		DEBUG_INIT_FULL_S("\n");
		DEBUG_INIT_FULL_S(" idx=0x");
		DEBUG_INIT_FULL_D(idx, 8);
		DEBUG_INIT_FULL_S("\n");

		/* Configuration for PEX only */
		if ((serdes_type != PEX0) && (serdes_type != PEX1) &&
		    (serdes_type != PEX2) && (serdes_type != PEX3))
			continue;

		if ((serdes_type != PEX0) &&
		    ((serdes_map[idx].serdes_mode == PEX_ROOT_COMPLEX_X4) ||
		     (serdes_map[idx].serdes_mode == PEX_END_POINT_X4))) {
			/* for PEX by4 - relevant for the first port only */
			continue;
		}

		pex_idx = serdes_type - PEX0;
		tmp = reg_read(PEX_DBG_STATUS_REG(pex_idx));

		first_busno = next_busno;
		if ((tmp & 0x7f) != 0x7e) {
			DEBUG_INIT_S("PCIe, Idx ");
			DEBUG_INIT_D(pex_idx, 1);
			DEBUG_INIT_S(": detected no link\n");
			continue;
		}

		next_busno++;
		temp_pex_reg = reg_read((PEX_CFG_DIRECT_ACCESS
					 (pex_idx, PEX_LINK_CAPABILITY_REG)));
		temp_pex_reg &= 0xf;
		if (temp_pex_reg != 0x2)
			continue;

		temp_reg = (reg_read(PEX_CFG_DIRECT_ACCESS(
					     pex_idx,
					     PEX_LINK_CTRL_STAT_REG)) &
			    0xf0000) >> 16;

		/* Check if the link established is GEN1 */
		DEBUG_INIT_FULL_S
			("Checking if the link established is gen1\n");
		if (temp_reg != 0x1)
			continue;

		pex_local_bus_num_set(pex_idx, first_busno);
		pex_local_dev_num_set(pex_idx, 1);
		DEBUG_INIT_FULL_S("PCIe, Idx ");
		DEBUG_INIT_FULL_D(pex_idx, 1);

		DEBUG_INIT_S(":** Link is Gen1, check the EP capability\n");
		/* link is Gen1, check the EP capability */
		addr = pex_config_read(pex_idx, first_busno, 0, 0, 0x34) & 0xff;
		DEBUG_INIT_FULL_C("pex_config_read: return addr=0x%x", addr, 4);
		if (addr == 0xff) {
			DEBUG_INIT_FULL_C
				("pex_config_read: return 0xff -->PCIe (%d): Detected No Link.",
				 pex_idx, 1);
			continue;
		}

		while ((pex_config_read(pex_idx, first_busno, 0, 0, addr)
			& 0xff) != 0x10) {
			addr = (pex_config_read(pex_idx, first_busno, 0,
						0, addr) & 0xff00) >> 8;
		}

		/* Check for Gen2 and above */
		if ((pex_config_read(pex_idx, first_busno, 0, 0,
				     addr + 0xc) & 0xf) < 0x2) {
			DEBUG_INIT_S("PCIe, Idx ");
			DEBUG_INIT_D(pex_idx, 1);
			DEBUG_INIT_S(": remains Gen1\n");
			continue;
		}

		tmp = reg_read(PEX_LINK_CTRL_STATUS2_REG(pex_idx));
		DEBUG_RD_REG(PEX_LINK_CTRL_STATUS2_REG(pex_idx), tmp);
		tmp &= ~(BIT(0) | BIT(1));
		tmp |= BIT(1);
		tmp |= BIT(6);	/* Select Deemphasize (-3.5d_b) */
		reg_write(PEX_LINK_CTRL_STATUS2_REG(pex_idx), tmp);
		DEBUG_WR_REG(PEX_LINK_CTRL_STATUS2_REG(pex_idx), tmp);

		tmp = reg_read(PEX_CTRL_REG(pex_idx));
		DEBUG_RD_REG(PEX_CTRL_REG(pex_idx), tmp);
		tmp |= BIT(10);
		reg_write(PEX_CTRL_REG(pex_idx), tmp);
		DEBUG_WR_REG(PEX_CTRL_REG(pex_idx), tmp);

		/*
		 * We need to wait 10ms before reading the PEX_DBG_STATUS_REG
		 * in order not to read the status of the former state
		 */
		mdelay(10);

		DEBUG_INIT_S("PCIe, Idx ");
		DEBUG_INIT_D(pex_idx, 1);
		DEBUG_INIT_S
			(": Link upgraded to Gen2 based on client capabilities\n");
	}

	/* Update pex DEVICE ID */
	ctrl_mode = sys_env_model_get();

	for (idx = 0; idx < count; idx++) {
		serdes_type = serdes_map[idx].serdes_type;
		/* configuration for PEX only */
		if ((serdes_type != PEX0) && (serdes_type != PEX1) &&
		    (serdes_type != PEX2) && (serdes_type != PEX3))
			continue;

		if ((serdes_type != PEX0) &&
		    ((serdes_map[idx].serdes_mode == PEX_ROOT_COMPLEX_X4) ||
		     (serdes_map[idx].serdes_mode == PEX_END_POINT_X4))) {
			/* for PEX by4 - relevant for the first port only */
			continue;
		}

		pex_idx = serdes_type - PEX0;
		dev_id = reg_read(PEX_CFG_DIRECT_ACCESS
				  (pex_idx, PEX_DEVICE_AND_VENDOR_ID));
		dev_id &= 0xffff;
		dev_id |= ((ctrl_mode << 16) & 0xffff0000);
		reg_write(PEX_CFG_DIRECT_ACCESS
			  (pex_idx, PEX_DEVICE_AND_VENDOR_ID), dev_id);
	}
	DEBUG_INIT_FULL_C("Update PEX Device ID ", ctrl_mode, 4);

	return MV_OK;
}

int pex_local_bus_num_set(u32 pex_if, u32 bus_num)
{
	u32 pex_status;

	DEBUG_INIT_FULL_S("\n### pex_local_bus_num_set ###\n");

	if (bus_num >= MAX_PEX_BUSSES) {
		DEBUG_INIT_C("pex_local_bus_num_set: Illegal bus number %d\n",
			     bus_num, 4);
		return MV_BAD_PARAM;
	}

	pex_status = reg_read(PEX_STATUS_REG(pex_if));
	pex_status &= ~PXSR_PEX_BUS_NUM_MASK;
	pex_status |=
	    (bus_num << PXSR_PEX_BUS_NUM_OFFS) & PXSR_PEX_BUS_NUM_MASK;
	reg_write(PEX_STATUS_REG(pex_if), pex_status);

	return MV_OK;
}

int pex_local_dev_num_set(u32 pex_if, u32 dev_num)
{
	u32 pex_status;

	DEBUG_INIT_FULL_S("\n### pex_local_dev_num_set ###\n");

	pex_status = reg_read(PEX_STATUS_REG(pex_if));
	pex_status &= ~PXSR_PEX_DEV_NUM_MASK;
	pex_status |=
	    (dev_num << PXSR_PEX_DEV_NUM_OFFS) & PXSR_PEX_DEV_NUM_MASK;
	reg_write(PEX_STATUS_REG(pex_if), pex_status);

	return MV_OK;
}

/*
 * pex_config_read - Read from configuration space
 *
 * DESCRIPTION:
 *       This function performs a 32 bit read from PEX configuration space.
 *       It supports both type 0 and type 1 of Configuration Transactions
 *       (local and over bridge). In order to read from local bus segment, use
 *       bus number retrieved from pex_local_bus_num_get(). Other bus numbers
 *       will result configuration transaction of type 1 (over bridge).
 *
 * INPUT:
 *       pex_if   - PEX interface number.
 *       bus      - PEX segment bus number.
 *       dev      - PEX device number.
 *       func     - Function number.
 *       reg_offs - Register offset.
 *
 * OUTPUT:
 *       None.
 *
 * RETURN:
 *       32bit register data, 0xffffffff on error
 */
u32 pex_config_read(u32 pex_if, u32 bus, u32 dev, u32 func, u32 reg_off)
{
	u32 pex_data = 0;
	u32 local_dev, local_bus;
	u32 pex_status;

	pex_status = reg_read(PEX_STATUS_REG(pex_if));
	local_dev =
	    ((pex_status & PXSR_PEX_DEV_NUM_MASK) >> PXSR_PEX_DEV_NUM_OFFS);
	local_bus =
	    ((pex_status & PXSR_PEX_BUS_NUM_MASK) >> PXSR_PEX_BUS_NUM_OFFS);

	/*
	 * In PCI Express we have only one device number
	 * and this number is the first number we encounter
	 * else that the local_dev
	 * spec pex define return on config read/write on any device
	 */
	if (bus == local_bus) {
		if (local_dev == 0) {
			/*
			 * if local dev is 0 then the first number we encounter
			 * after 0 is 1
			 */
			if ((dev != 1) && (dev != local_dev))
				return MV_ERROR;
		} else {
			/*
			 * if local dev is not 0 then the first number we
			 * encounter is 0
			 */
			if ((dev != 0) && (dev != local_dev))
				return MV_ERROR;
		}
	}

	/* Creating PEX address to be passed */
	pex_data = (bus << PXCAR_BUS_NUM_OFFS);
	pex_data |= (dev << PXCAR_DEVICE_NUM_OFFS);
	pex_data |= (func << PXCAR_FUNC_NUM_OFFS);
	/* Legacy register space */
	pex_data |= (reg_off & PXCAR_REG_NUM_MASK);
	/* Extended register space */
	pex_data |= (((reg_off & PXCAR_REAL_EXT_REG_NUM_MASK) >>
		      PXCAR_REAL_EXT_REG_NUM_OFFS) << PXCAR_EXT_REG_NUM_OFFS);
	pex_data |= PXCAR_CONFIG_EN;

	/* Write the address to the PEX configuration address register */
	reg_write(PEX_CFG_ADDR_REG(pex_if), pex_data);

	/*
	 * In order to let the PEX controller absorbed the address
	 * of the read transaction we perform a validity check that
	 * the address was written
	 */
	if (pex_data != reg_read(PEX_CFG_ADDR_REG(pex_if)))
		return MV_ERROR;

	/* Cleaning Master Abort */
	reg_bit_set(PEX_CFG_DIRECT_ACCESS(pex_if, PEX_STATUS_AND_COMMAND),
		    PXSAC_MABORT);
	/* Read the Data returned in the PEX Data register */
	pex_data = reg_read(PEX_CFG_DATA_REG(pex_if));

	DEBUG_INIT_FULL_C(" --> ", pex_data, 4);

	return pex_data;
}
