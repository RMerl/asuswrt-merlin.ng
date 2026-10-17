/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2001
 * Erik Theisen,  Wave 7 Optics, etheisen@mindspring.com.
 */

/*
 * Watchdog functions and macros.
 */
#ifndef _WATCHDOG_H_
#define _WATCHDOG_H_

#include <cyclic.h>

/*
 * Reset the watchdog timer, always returns 0
 *
 * This function is here since it is shared between board_f() and board_r(),
 * and the legacy arch/<arch>/board.c code.
 */
int init_func_watchdog_reset(void);

#if defined(CONFIG_WATCHDOG) || defined(CONFIG_HW_WATCHDOG)
#define INIT_FUNC_WATCHDOG_INIT	init_func_watchdog_init,
#define INIT_FUNC_WATCHDOG_RESET	init_func_watchdog_reset,
#else
#define INIT_FUNC_WATCHDOG_INIT
#define INIT_FUNC_WATCHDOG_RESET
#endif

#if defined(CONFIG_HW_WATCHDOG) && defined(CONFIG_WATCHDOG)
#  error "Configuration error: CONFIG_HW_WATCHDOG and CONFIG_WATCHDOG can't be used together."
#endif

/*
 * Prototypes from $(CPU)/cpu.c.
 */

#if defined(CONFIG_HW_WATCHDOG) || defined(CONFIG_WATCHDOG)
	void hw_watchdog_init(void);
#endif

#if defined(CONFIG_MPC85xx)
	void init_85xx_watchdog(void);
#endif
#endif /* _WATCHDOG_H_ */
