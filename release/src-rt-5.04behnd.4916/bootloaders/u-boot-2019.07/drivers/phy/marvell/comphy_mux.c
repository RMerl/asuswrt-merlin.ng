// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015-2016 Marvell International Ltd.
 */

#include <common.h>
#include <asm/io.h>

#include "comphy_core.h"
#include "comphy_hpipe.h"

/*
 * comphy_mux_check_config()
 * description: this function passes over the COMPHY lanes and check if the type
 *              is valid for specific lane. If the type is not valid,
 *              the function update the struct and set the type of the lane as
 *              PHY_TYPE_UNCONNECTED
 */
static void comphy_mux_check_config(struct comphy_mux_data *mux_data,
		struct comphy_map *comphy_map_data, int comphy_max_lanes)
{
	struct comphy_mux_options *mux_opt;
	int lane, opt, valid;

	debug_enter();

	for (lane = 0; lane < comphy_max_lanes;
	     lane++, comphy_map_data++, mux_data++) {
		/* Don't check ignored COMPHYs */
		if (comphy_map_data->type == PHY_TYPE_IGNORE)
			continue;

		mux_opt = mux_data->mux_values;
		for (opt = 0, valid = 0; opt < mux_data->max_lane_values;
		     opt++, mux_opt++) {
			if (mux_opt->type == comphy_map_data->type) {
				valid = 1;
				break;
			}
		}
		if (valid == 0) {
			debug("lane number %d, had invalid type %d\n",
			      lane, comphy_map_data->type);
			debug("set lane %d as type %d\n", lane,
			      PHY_TYPE_UNCONNECTED);
			comphy_map_data->type = PHY_TYPE_UNCONNECTED;
		} else {
			debug("lane number %d, has type %d\n",
			      lane, comphy_map_data->type);
		}
	}

	debug_exit();
}

static u32 comphy_mux_get_mux_value(struct comphy_mux_data *mux_data,
				    u32 type, int lane)
{
	struct comphy_mux_options *mux_opt;
	int opt;
	u32 value = 0;

	debug_enter();

	mux_opt = mux_data->mux_values;
	for (opt = 0 ; opt < mux_data->max_lane_values; opt++, mux_opt++) {
		if (mux_opt->type == type) {
			value = mux_opt->mux_value;
			break;
		}
	}

	debug_exit();

	return value;
}

static void comphy_mux_reg_write(struct comphy_mux_data *mux_data,
				 struct comphy_map *comphy_map_data,
				 int comphy_max_lanes,
				 void __iomem *selector_base,
				 const fdt32_t *mux_lane_order, u32 bitcount)
{
	u32 lane, value, offset, mask;

	debug_enter();

	for (lane = 0; lane < comphy_max_lanes;
	     lane++, comphy_map_data++, mux_data++) {
		if (comphy_map_data->type == PHY_TYPE_IGNORE)
			continue;

		/*
		 * if the order of nodes in selector base register is
		 * nontrivial, use mapping from mux_lane_order
		 */
		if (mux_lane_order)
			offset = fdt32_to_cpu(mux_lane_order[lane]) * bitcount;
		else
			offset = lane * bitcount;

		mask = (((1 << bitcount) - 1) << offset);
		value = (comphy_mux_get_mux_value(mux_data,
						  comphy_map_data->type,
						  lane) << offset);
		reg_set(selector_base, value, mask);
	}

	debug_exit();
}

void comphy_mux_init(struct chip_serdes_phy_config *chip_cfg,
		     struct comphy_map *comphy_map_data,
		     void __iomem *selector_base)
{
	struct comphy_mux_data *mux_data;
	const fdt32_t *mux_lane_order;
	u32 mux_bitcount;
	u32 comphy_max_lanes;

	debug_enter();

	comphy_max_lanes = chip_cfg->comphy_lanes_count;
	mux_data = chip_cfg->mux_data;
	mux_lane_order = chip_cfg->comphy_mux_lane_order;
	mux_bitcount = chip_cfg->comphy_mux_bitcount;

	/* check if the configuration is valid */
	comphy_mux_check_config(mux_data, comphy_map_data, comphy_max_lanes);
	/* Init COMPHY selectors */
	comphy_mux_reg_write(mux_data, comphy_map_data, comphy_max_lanes,
			     selector_base, mux_lane_order, mux_bitcount);

	debug_exit();
}
