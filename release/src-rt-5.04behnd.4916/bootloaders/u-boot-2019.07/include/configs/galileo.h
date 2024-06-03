/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015, Bin Meng <bmeng.cn@gmail.com>
 */

/*
 * board/config.h - configuration options, board specific
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <configs/x86-common.h>

#define CONFIG_SYS_MONITOR_LEN		(1 << 20)

/* ns16550 UART is memory-mapped in Quark SoC */
#undef  CONFIG_SYS_NS16550_PORT_MAPPED

#define CONFIG_STD_DEVICES_SETTINGS	"stdin=serial\0" \
					"stdout=serial\0" \
					"stderr=serial\0"

/* 10/100M Ethernet support */
#define CONFIG_DESIGNWARE_ETH
#define CONFIG_DW_ALTDESCRIPTOR

/* Environment configuration */
#define CONFIG_ENV_SECT_SIZE		0x1000
#define CONFIG_ENV_OFFSET		0

#endif	/* __CONFIG_H */
