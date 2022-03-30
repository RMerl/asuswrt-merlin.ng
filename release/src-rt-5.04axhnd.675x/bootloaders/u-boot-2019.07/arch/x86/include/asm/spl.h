/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2017 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef __asm_spl_h
#define __asm_spl_h

#define CONFIG_SPL_BOARD_LOAD_IMAGE

enum {
	BOOT_DEVICE_SPI		= 10,
	BOOT_DEVICE_BOARD,
	BOOT_DEVICE_CROS_VBOOT,
};

void jump_to_spl(ulong entry);

#endif
