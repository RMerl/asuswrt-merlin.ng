/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2017 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef __asm_spl_h
#define __asm_spl_h

enum {
	BOOT_DEVICE_SPI_MMAP	= 10,
	BOOT_DEVICE_FAST_SPI,
	BOOT_DEVICE_CROS_VBOOT,
};

void jump_to_spl(ulong entry);

#endif
