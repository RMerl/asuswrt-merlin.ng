/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2020 Google LLC
 */

#ifndef __DM_SIMPLE_BUS_H
#define __DM_SIMPLE_BUS_H

struct simple_bus_plat {
	fdt_addr_t base;
	fdt_size_t size;
	fdt_addr_t target;
};

#endif
