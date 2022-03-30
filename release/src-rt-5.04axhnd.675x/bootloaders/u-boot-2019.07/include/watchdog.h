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

#if !defined(__ASSEMBLY__)
/*
 * Reset the watchdog timer, always returns 0
 *
 * This function is here since it is shared between board_f() and board_r(),
 * and the legacy arch/<arch>/board.c code.
 */
int init_func_watchdog_reset(void);
#endif

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
 * Hardware watchdog
 */
#ifdef CONFIG_HW_WATCHDOG
	#if defined(__ASSEMBLY__)
		#define WATCHDOG_RESET bl hw_watchdog_reset
	#else
		extern void hw_watchdog_reset(void);

		#define WATCHDOG_RESET hw_watchdog_reset
	#endif /* __ASSEMBLY__ */
#else
	/*
	 * Maybe a software watchdog?
	 */
	#if defined(CONFIG_WATCHDOG)
		#if defined(__ASSEMBLY__)
			#define WATCHDOG_RESET bl watchdog_reset
		#else
			/* Don't require the watchdog to be enabled in SPL */
			#if defined(CONFIG_SPL_BUILD) &&		\
				!defined(CONFIG_SPL_WATCHDOG_SUPPORT)
				#define WATCHDOG_RESET() {}
			#else
				extern void watchdog_reset(void);

				#define WATCHDOG_RESET watchdog_reset
			#endif
		#endif
	#else
		/*
		 * No hardware or software watchdog.
		 */
		#if defined(__ASSEMBLY__)
			#define WATCHDOG_RESET /*XXX DO_NOT_DEL_THIS_COMMENT*/
		#else
			#define WATCHDOG_RESET() {}
		#endif /* __ASSEMBLY__ */
	#endif /* CONFIG_WATCHDOG && !__ASSEMBLY__ */
#endif /* CONFIG_HW_WATCHDOG */

/*
 * Prototypes from $(CPU)/cpu.c.
 */

#if defined(CONFIG_HW_WATCHDOG) && !defined(__ASSEMBLY__)
	void hw_watchdog_init(void);
#endif

#if defined(CONFIG_MPC85xx) && !defined(__ASSEMBLY__)
	void init_85xx_watchdog(void);
#endif
#endif /* _WATCHDOG_H_ */
