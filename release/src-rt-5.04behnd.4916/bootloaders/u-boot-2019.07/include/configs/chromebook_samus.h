/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2011 The Chromium OS Authors.
 * (C) Copyright 2008
 * Graeme Russ, graeme.russ@gmail.com.
 */

/*
 * board/config.h - configuration options, board specific
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <configs/x86-common.h>
#include <configs/x86-chromebook.h>

#undef CONFIG_STD_DEVICES_SETTINGS
#define CONFIG_STD_DEVICES_SETTINGS     "stdin=usbkbd,i8042-kbd,serial\0" \
					"stdout=vidconsole,serial\0" \
					"stderr=vidconsole,serial\0"

#define CONFIG_ENV_SECT_SIZE		0x1000
#define CONFIG_ENV_OFFSET		0x003f8000

#define CONFIG_TPL_TEXT_BASE		0xfffd8000

#endif	/* __CONFIG_H */
