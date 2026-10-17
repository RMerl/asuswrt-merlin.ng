/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2011 The ChromiumOS Authors.  All rights reserved.
 *
 * Taken from the coreboot version
 */

#ifndef __COREBOOT_TIMESTAMP_H__
#define __COREBOOT_TIMESTAMP_H__

#include <asm/cb_sysinfo.h>

void timestamp_init(void);
void timestamp_add(enum timestamp_id id, uint64_t ts_time);
void timestamp_add_now(enum timestamp_id id);

/**
 * timestamp_add_to_bootstage - Add important coreboot timestamps to bootstage
 *
 * Return: 0 if ok, -1 if no timestamps were found
 */
int timestamp_add_to_bootstage(void);

#endif
