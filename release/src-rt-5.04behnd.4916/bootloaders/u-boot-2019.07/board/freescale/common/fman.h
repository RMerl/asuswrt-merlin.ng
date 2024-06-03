/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2011 Freescale Semiconductor, Inc.
 */

#ifndef __FMAN_BOARD_HELPER__
#define __FMAN_BOARD_HELPER__

int fdt_set_phy_handle(void *fdt, char *compat, phys_addr_t addr,
			const char *alias);

enum srds_prtcl serdes_device_from_fm_port(enum fm_port port);

#endif
