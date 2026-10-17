/* SPDX-License-Identifier: GPL-2.0 */
/*
 * (C) Copyright 2020 Xilinx, Inc.
 * Michal Simek <michal.simek@amd.com>
 */

#ifndef _BOARD_XILINX_COMMON_BOARD_H
#define _BOARD_XILINX_COMMON_BOARD_H

int board_late_init_xilinx(void);

int xilinx_read_eeprom(void);

char *board_name_decode(void);

bool board_detection(void);

char *soc_name_decode(void);

bool soc_detection(void);
#endif /* BOARD_XILINX_COMMON_BOARD_H */
