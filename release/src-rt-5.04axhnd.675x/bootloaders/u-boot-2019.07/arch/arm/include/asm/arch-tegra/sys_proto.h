/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2010,2011
 * NVIDIA Corporation <www.nvidia.com>
 */

#ifndef _SYS_PROTO_H_
#define _SYS_PROTO_H_

void invalidate_dcache(void);

/**
 * tegra_board_id() - Get the board iD
 *
 * @return a board ID, or -ve on error
 */
int tegra_board_id(void);

/**
 * tegra_lcd_pmic_init() - Set up the PMIC for a board
 *
 * @board_id: Board ID which may be used to select LCD type
 * @return 0 if OK, -ve on error
 */
int tegra_lcd_pmic_init(int board_id);

/**
 * nvidia_board_init() - perform any board-specific init
 *
 * @return 0 if OK, -ve on error
 */
int nvidia_board_init(void);

#endif
