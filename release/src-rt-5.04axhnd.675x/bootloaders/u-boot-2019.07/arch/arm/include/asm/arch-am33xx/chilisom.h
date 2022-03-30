/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2017 Grinn
 */

#ifndef __ARCH_ARM_MACH_CHILISOM_SOM_H__
#define __ARCH_ARM_MACH_CHILISOM_SOM_H__

#ifndef CONFIG_SKIP_LOWLEVEL_INIT
void chilisom_enable_pin_mux(void);
void chilisom_spl_board_init(void);
#endif

#endif
