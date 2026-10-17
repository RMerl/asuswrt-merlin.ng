/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2011 The Chromium OS Authors.
 */

#ifndef __INITCALL_H
#define __INITCALL_H

#include <asm/types.h>
#include <event.h>

_Static_assert(EVT_COUNT < 256, "Can only support 256 event types with 8 bits");

/**
 * init_fnc_t - Init function
 *
 * Return: 0 if OK -ve on error
 */
typedef int (*init_fnc_t)(void);

/* Top bit indicates that the initcall is an event */
#define INITCALL_IS_EVENT	GENMASK(BITS_PER_LONG - 1, 8)
#define INITCALL_EVENT_TYPE	GENMASK(7, 0)

#define INITCALL_EVENT(_type)	(void *)((_type) | INITCALL_IS_EVENT)

/**
 * initcall_run_list() - Run through a list of function calls
 *
 * This calls functions one after the other, stopping at the first error, or
 * when NULL is obtained.
 *
 * @init_sequence: NULL-terminated init sequence to run
 * Return: 0 if OK, or -ve error code from the first failure
 */
int initcall_run_list(const init_fnc_t init_sequence[]);

#endif
