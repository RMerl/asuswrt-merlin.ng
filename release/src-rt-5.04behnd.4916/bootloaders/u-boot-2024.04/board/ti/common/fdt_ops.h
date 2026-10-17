/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Library to support common device tree manipulation for TI EVMs
 *
 * Copyright (C) 2024 Texas Instruments Incorporated - https://www.ti.com
 */

#ifndef __FDT_OPS_H
#define __FDT_OPS_H

#define TI_BOARD_NAME_MAX 20
#define TI_FDT_FILE_MAX 200

/**
 *  struct ti_fdt_map - mapping of device tree blob name to board name
 *  @board_name: board_name up to TI_BOARD_NAME_MAX long
 *  @fdt_file_name: device tree blob name as described by kernel
 */
struct ti_fdt_map {
	const char *board_name;
	char *fdt_file_name;
};

/**
 * ti_set_fdt_env  - Find the correct device tree file name based on the
 * board name and set 'fdtfile' env variable with correct folder
 * structure appropriate to the architecture and Linux kernel's
 * 'make install_dtbs' conventions. This function is invoked typically
 * as part of board_late_init.
 *
 * fdt name is picked by:
 * a) If a board name match is found, use the match
 * b) If not, CONFIG_DEFAULT_FDT_FILE (Boot OS device tree) if that is defined
 *    and not null
 * c) If not, Use CONFIG_DEFAULT_DEVICE_TREE (DT control for bootloader)
 *
 * @board_name: match to search with (max of TI_BOARD_NAME_MAX chars)
 * @fdt_map: NULL terminated array of device tree file name matches.
 */
void ti_set_fdt_env(const char *board_name, struct ti_fdt_map *fdt_map);

#endif /* __FDT_OPS_H */
