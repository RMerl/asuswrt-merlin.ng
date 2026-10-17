/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019 Google LLC
 */

#ifndef __ASM_FSP_API_H
#define __ASM_FSP_API_H

#include <linux/linkage.h>

enum fsp_phase {
	/* Notification code for post PCI enuermation */
	INIT_PHASE_PCI		= 0x20,
	/*
	 * Notification code before transferring control to the payload.
	 * This is issued at the end of init before starting main(), i.e.
	 * the command line / boot script.
	 */
	INIT_PHASE_BOOT		= 0x40,
	/*
	 * Notification code before existing boot services. This is issued
	 * just before removing devices and booting the kernel.
	 */
	INIT_PHASE_END_FIRMWARE	= 0xf0,
};

struct fsp_notify_params {
	/* Notification phase used for NotifyPhase API */
	enum fsp_phase	phase;
};

/* FspNotify API function prototype */
typedef asmlinkage u32 (*fsp_notify_f)(struct fsp_notify_params *params);

#endif
