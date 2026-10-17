/* SPDX-License-Identifier:     GPL-2.0+ */
/*
 * Copyright 2018-2019 NXP
 */

#ifndef SC_TIMER_API_H
#define SC_TIMER_API_H

/* Defines */

/* Defines for type widths */
#define SC_TIMER_ACTION_W   3U      /* Width of sc_timer_wdog_action_t */

/* Defines for sc_timer_wdog_action_t */
#define SC_TIMER_WDOG_ACTION_PARTITION      0U   /* Reset partition */
#define SC_TIMER_WDOG_ACTION_WARM           1U   /* Warm reset system */
#define SC_TIMER_WDOG_ACTION_COLD           2U   /* Cold reset system */
#define SC_TIMER_WDOG_ACTION_BOARD          3U   /* Reset board */
#define SC_TIMER_WDOG_ACTION_IRQ            4U   /* Only generate IRQs */

/* Types */

/*
 * This type is used to configure the watchdog action.
 */
typedef u8 sc_timer_wdog_action_t;

/*
 * This type is used to declare a watchdog time value in milliseconds.
 */
typedef u32 sc_timer_wdog_time_t;

#endif /* SC_TIMER_API_H */
