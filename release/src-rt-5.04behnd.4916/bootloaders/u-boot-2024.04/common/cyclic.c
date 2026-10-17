// SPDX-License-Identifier: GPL-2.0+
/*
 * A general-purpose cyclic execution infrastructure, to allow "small"
 * (run-time wise) functions to be executed at a specified frequency.
 * Things like LED blinking or watchdog triggering are examples for such
 * tasks.
 *
 * Copyright (C) 2022 Stefan Roese <sr@denx.de>
 */

#include <cyclic.h>
#include <log.h>
#include <malloc.h>
#include <time.h>
#include <linux/errno.h>
#include <linux/list.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

void hw_watchdog_reset(void);

struct hlist_head *cyclic_get_list(void)
{
	/* Silence "discards 'volatile' qualifier" warning. */
	return (struct hlist_head *)&gd->cyclic_list;
}

struct cyclic_info *cyclic_register(cyclic_func_t func, uint64_t delay_us,
				    const char *name, void *ctx)
{
	struct cyclic_info *cyclic;

	cyclic = calloc(1, sizeof(struct cyclic_info));
	if (!cyclic) {
		pr_debug("Memory allocation error\n");
		return NULL;
	}

	/* Store values in struct */
	cyclic->func = func;
	cyclic->ctx = ctx;
	cyclic->name = strdup(name);
	cyclic->delay_us = delay_us;
	cyclic->start_time_us = timer_get_us();
	hlist_add_head(&cyclic->list, cyclic_get_list());

	return cyclic;
}

int cyclic_unregister(struct cyclic_info *cyclic)
{
	hlist_del(&cyclic->list);
	free(cyclic);

	return 0;
}

void cyclic_run(void)
{
	struct cyclic_info *cyclic;
	struct hlist_node *tmp;
	uint64_t now, cpu_time;

	/* Prevent recursion */
	if (gd->flags & GD_FLG_CYCLIC_RUNNING)
		return;

	gd->flags |= GD_FLG_CYCLIC_RUNNING;
	hlist_for_each_entry_safe(cyclic, tmp, cyclic_get_list(), list) {
		/*
		 * Check if this cyclic function needs to get called, e.g.
		 * do not call the cyclic func too often
		 */
		now = timer_get_us();
		if (time_after_eq64(now, cyclic->next_call)) {
			/* Call cyclic function and account it's cpu-time */
			cyclic->next_call = now + cyclic->delay_us;
			cyclic->func(cyclic->ctx);
			cyclic->run_cnt++;
			cpu_time = timer_get_us() - now;
			cyclic->cpu_time_us += cpu_time;

			/* Check if cpu-time exceeds max allowed time */
			if ((cpu_time > CONFIG_CYCLIC_MAX_CPU_TIME_US) &&
			    (!cyclic->already_warned)) {
				pr_err("cyclic function %s took too long: %lldus vs %dus max\n",
				       cyclic->name, cpu_time,
				       CONFIG_CYCLIC_MAX_CPU_TIME_US);

				/*
				 * Don't disable this function, just warn once
				 * about this exceeding CPU time usage
				 */
				cyclic->already_warned = true;
			}
		}
	}
	gd->flags &= ~GD_FLG_CYCLIC_RUNNING;
}

void schedule(void)
{
	/* The HW watchdog is not integrated into the cyclic IF (yet) */
	if (IS_ENABLED(CONFIG_HW_WATCHDOG))
		hw_watchdog_reset();

	/*
	 * schedule() might get called very early before the cyclic IF is
	 * ready. Make sure to only call cyclic_run() when it's initalized.
	 */
	if (gd)
		cyclic_run();
}

int cyclic_unregister_all(void)
{
	struct cyclic_info *cyclic;
	struct hlist_node *tmp;

	hlist_for_each_entry_safe(cyclic, tmp, cyclic_get_list(), list)
		cyclic_unregister(cyclic);

	return 0;
}
