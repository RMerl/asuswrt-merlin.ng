/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2013, 2014 TQ Systems
 * Author: Markus Niebel <markus.niebel@tq-group.com>
 */

#ifndef __TQMA6_BB__
#define __TQMA6_BB__

#include <common.h>

int tqma6_bb_board_mmc_getwp(struct mmc *mmc);
int tqma6_bb_board_mmc_getcd(struct mmc *mmc);
int tqma6_bb_board_mmc_init(bd_t *bis);

int tqma6_bb_board_early_init_f(void);
int tqma6_bb_board_init(void);
int tqma6_bb_board_late_init(void);
int tqma6_bb_checkboard(void);

const char *tqma6_bb_get_boardname(void);
/*
 * Device Tree Support
 */
#if defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_OF_LIBFDT)
void tqma6_bb_ft_board_setup(void *blob, bd_t *bd);
#endif /* defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_OF_LIBFDT) */

#endif
