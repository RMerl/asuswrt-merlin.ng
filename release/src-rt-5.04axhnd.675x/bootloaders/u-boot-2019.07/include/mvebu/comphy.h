/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015-2016 Marvell International Ltd.
 */

#ifndef _MVEBU_COMPHY_H_
#define _MVEBU_COMPHY_H_

#include <dt-bindings/comphy/comphy_data.h>

struct comphy_map {
	u32 type;
	u32 speed;
	u32 invert;
	bool clk_src;
	bool end_point;
};

int comphy_update_map(struct comphy_map *serdes_map, int count);

#endif /* _MVEBU_COMPHY_H_ */

