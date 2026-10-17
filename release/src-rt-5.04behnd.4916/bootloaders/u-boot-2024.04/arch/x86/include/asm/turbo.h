/* SPDX-License-Identifier: GPL-2.0 */
/*
 * From coreboot file of the same name
 *
 * Copyright (C) 2011 The ChromiumOS Authors. All rights reserved.
 */

#ifndef _ASM_TURBO_H
#define _ASM_TURBO_H

#define CPUID_LEAF_PM		6
#define PM_CAP_TURBO_MODE	(1 << 1)

enum {
	TURBO_UNKNOWN,
	TURBO_UNAVAILABLE,
	TURBO_DISABLED,
	TURBO_ENABLED,
};

/* Return current turbo state */
int turbo_get_state(void);

/* Enable turbo */
void turbo_enable(void);

#endif
