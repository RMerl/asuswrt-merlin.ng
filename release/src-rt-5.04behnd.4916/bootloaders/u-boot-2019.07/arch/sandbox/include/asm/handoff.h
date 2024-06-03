/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Architecture-specific SPL handoff information for sandbox
 *
 * Copyright 2018 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef __handoff_h
#define __handoff_h

#define TEST_HANDOFF_MAGIC	0x14f93c7b

struct arch_spl_handoff {
	ulong	magic;		/* Used for testing */
};

#endif
