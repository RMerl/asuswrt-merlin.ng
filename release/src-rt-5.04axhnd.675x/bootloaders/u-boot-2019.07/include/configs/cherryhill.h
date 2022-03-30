/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2017, Bin Meng <bmeng.cn@gmail.com>
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <configs/x86-common.h>

#define CONFIG_SYS_MONITOR_LEN		(2 << 20)

#define CONFIG_STD_DEVICES_SETTINGS	"stdin=usbkbd,serial\0" \
					"stdout=vidconsole,serial\0" \
					"stderr=vidconsole,serial\0"

/* Environment configuration */
#define CONFIG_ENV_SECT_SIZE		0x10000
#define CONFIG_ENV_OFFSET		0x005f0000

#endif	/* __CONFIG_H */
