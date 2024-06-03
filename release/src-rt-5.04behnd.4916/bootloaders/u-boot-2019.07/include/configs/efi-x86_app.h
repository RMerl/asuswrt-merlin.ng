/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2015 Google, Inc
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <configs/x86-common.h>

#undef CONFIG_TPM_TIS_BASE_ADDRESS

#define CONFIG_STD_DEVICES_SETTINGS     "stdin=usbkbd,vga,serial\0" \
					"stdout=vga,serial\0" \
					"stderr=vga,serial\0"

#endif
