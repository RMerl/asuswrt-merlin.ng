/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * A general-purpose cyclic execution infrastructure, to allow "small"
 * (run-time wise) functions to be executed at a specified frequency.
 * Things like LED blinking or watchdog triggering are examples for such
 * tasks.
 *
 * Copyright (C) 2022 Stefan Roese <sr@denx.de>
 */

#ifndef __cyclic_h
#define __cyclic_h

#include <linux/list.h>
#include <asm/types.h>

/**
 * struct cyclic_info - Information about cyclic execution function
 *
 * @func: Function to call periodically
 * @ctx: Context pointer to get passed to this function
 * @name: Name of the cyclic function, e.g. shown in the commands
 * @delay_ns: Delay is ns after which this function shall get executed
 * @start_time_us: Start time in us, when this function started its execution
 * @cpu_time_us: Total CPU time of this function
 * @run_cnt: Counter of executions occurances
 * @next_call: Next time in us, when the function shall be executed again
 * @list: List node
 * @already_warned: Flag that we've warned about exceeding CPU time usage
 */
struct cyclic_info {
	void (*func)(void *ctx);
	void *ctx;
	char *name;
	uint64_t delay_us;
	uint64_t start_time_us;
	uint64_t cpu_time_us;
	uint64_t run_cnt;
	uint64_t next_call;
	struct hlist_node list;
	bool already_warned;
};

/** Function type for cyclic functions */
typedef void (*cyclic_func_t)(void *ctx);

#if defined(CONFIG_CYCLIC)
/**
 * cyclic_register - Register a new cyclic function
 *
 * @func: Function to call periodically
 * @delay_us: Delay is us after which this function shall get executed
 * @name: Cyclic function name/id
 * @ctx: Context to pass to the function
 * @return: pointer to cyclic_struct if OK, NULL on error
 */
struct cyclic_info *cyclic_register(cyclic_func_t func, uint64_t delay_us,
				    const char *name, void *ctx);

/**
 * cyclic_unregister - Unregister a cyclic function
 *
 * @cyclic: Pointer to cyclic_struct of the function that shall be removed
 * @return: 0 if OK, -ve on error
 */
int cyclic_unregister(struct cyclic_info *cyclic);

/**
 * cyclic_unregister_all() - Clean up cyclic functions
 *
 * This removes all cyclic functions
 */
int cyclic_unregister_all(void);

/**
 * cyclic_get_list() - Get cyclic list pointer
 *
 * Return the cyclic list pointer
 *
 * @return: pointer to cyclic_list
 */
struct hlist_head *cyclic_get_list(void);

/**
 * cyclic_run() - Interate over all registered cyclic functions
 *
 * Interate over all registered cyclic functions and if the it's function
 * needs to be executed, then call into these registered functions.
 */
void cyclic_run(void);

/**
 * schedule() - Schedule all potentially waiting tasks
 *
 * Basically a wrapper for cyclic_run(), pontentially enhanced by some
 * other parts, that need to get handled periodically.
 */
void schedule(void);
#else
static inline struct cyclic_info *cyclic_register(cyclic_func_t func,
						  uint64_t delay_us,
						  const char *name,
						  void *ctx)
{
	return NULL;
}

static inline int cyclic_unregister(struct cyclic_info *cyclic)
{
	return 0;
}

static inline void cyclic_run(void)
{
}

static inline void schedule(void)
{
}

static inline int cyclic_unregister_all(void)
{
	return 0;
}
#endif

#endif
