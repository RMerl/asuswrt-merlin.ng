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

u8 selectors_serdes_rev1_map[LAST_SERDES_TYPE][MAX_SERDES_LANES] = {
	/* 0  1    2    3    4    5 */
	{0x1, 0x1, NA,  NA,  NA,  NA},		/* PEX0 */
	{NA,  0x2, 0x1, NA,  0x1, NA},		/* PEX1 */
	{NA,  NA,  0x2, NA,  NA,  0x1},		/* PEX2 */
	{NA,  NA,  NA,  0x1, NA,  NA},		/* PEX3 */
	{0x2, 0x3, NA,  NA,  NA,  NA},		/* SATA0 */
	{NA,  NA,  0x3, NA,  0x2, NA},		/* SATA1 */
	{NA,  NA,  NA,  NA,  0x6, 0x2},		/* SATA2 */
	{NA,  NA,  NA,  0x3, NA,  NA},		/* SATA3 */
	{0x3, 0x4, NA,  NA,  NA,  NA},		/* SGMII0 */
	{NA,  0x5, 0x4, NA,  0x3, NA},		/* SGMII1 */
	{NA,  NA,  NA,  0x4, NA,  0x3},		/* SGMII2 */
	{NA,  0x7, NA,  NA,  NA,  NA},		/* QSGMII */
	{NA,  0x6, NA,  NA,  0x4, NA},		/* USB3_HOST0 */
	{NA,  NA,  NA,  0x5, NA,  0x4},		/* USB3_HOST1 */
	{NA,  NA,  NA,  0x6, 0x5, 0x5},		/* USB3_DEVICE */
	{0x0, 0x0, 0x0, 0x0, 0x0, 0x0}		/* DEFAULT_SERDES */
};

int hws_serdes_seq_init(void)
{
	DEBUG_INIT_FULL_S("\n### serdes_seq_init ###\n");

	if (hws_serdes_seq_db_init() != MV_OK) {
		printf("hws_serdes_seq_init: Error: Serdes initialization fail\n");
		return MV_FAIL;
	}

	return MV_OK;
}

int serdes_power_up_ctrl_ext(u32 serdes_num, int serdes_power_up,
			     enum serdes_type serdes_type,
			     enum serdes_speed baud_rate,
			     enum serdes_mode serdes_mode,
			     enum ref_clock ref_clock)
{
	return MV_NOT_SUPPORTED;
}

u32 hws_serdes_silicon_ref_clock_get(void)
{
	DEBUG_INIT_FULL_S("\n### hws_serdes_silicon_ref_clock_get ###\n");

	return REF_CLOCK_25MHZ;
}

u32 hws_serdes_get_max_lane(void)
{
	switch (sys_env_device_id_get()) {
	case MV_6811:		/* A381/A3282: 6811/6821: single/dual cpu */
		return 4;
	case MV_6810:
		return 5;
	case MV_6820:
	case MV_6828:
		return 6;
	default:		/* not the right module */
		printf("%s: Device ID Error, using 4 SerDes lanes\n",
		       __func__);
		return 4;
	}
	return 6;
}

int hws_is_serdes_active(u8 lane_num)
{
	int ret = 1;

	/* Maximum lane count for A388 (6828) is 6 */
	if (lane_num > 6)
		ret = 0;

	/* 4th Lane (#4 on Device 6810 is not Active */
	if (sys_env_device_id_get() == MV_6810 && lane_num == 4) {
		printf("%s: Error: Lane#4 on Device 6810 is not Active.\n",
		       __func__);
		return 0;
	}

	/*
	 * 6th Lane (#5) on Device 6810 is Active, even though 6810
	 * has only 5 lanes
	 */
	if (sys_env_device_id_get() == MV_6810 && lane_num == 5)
		return 1;

	if (lane_num >= hws_serdes_get_max_lane())
		ret = 0;

	return ret;
}

int hws_get_ext_base_addr(u32 serdes_num, u32 base_addr, u32 unit_base_offset,
			  u32 *unit_base_reg, u32 *unit_offset)
{
	*unit_base_reg = base_addr;
	*unit_offset = unit_base_offset;

	return MV_OK;
}

/*
 * hws_serdes_get_phy_selector_val
 *
 * DESCRIPTION: Get the mapping of Serdes Selector values according to the
 *              Serdes revision number
 * INPUT:    serdes_num - Serdes number
 *           serdes_type - Serdes type
 * OUTPUT: None
 * RETURN:
 *       Mapping of Serdes Selector values
 */
u32 hws_serdes_get_phy_selector_val(int serdes_num,
				    enum serdes_type serdes_type)
{
	if (serdes_type >= LAST_SERDES_TYPE)
		return 0xff;

	if (hws_ctrl_serdes_rev_get() == MV_SERDES_REV_1_2) {
		return selectors_serdes_rev1_map
			[serdes_type][serdes_num];
	} else
		return selectors_serdes_rev2_map
			[serdes_type][serdes_num];
}

u32 hws_get_physical_serdes_num(u32 serdes_num)
{
	if ((serdes_num == 4) && (sys_env_device_id_get() == MV_6810)) {
		/*
		 * For 6810, there are 5 Serdes and Serdes Num 4 doesn't
		 * exist. Instead Serdes Num 5 is connected.
		 */
		return 5;
	} else {
		return serdes_num;
	}
}
