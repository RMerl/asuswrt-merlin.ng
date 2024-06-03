/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2014 Broadcom Corporation.
 */

#ifndef __ARCH_CONFIGS_H
#define __ARCH_CONFIGS_H

#include <asm/iproc-common/configs.h>

/* uArchitecture specifics */

/* Serial Info */
/* no padding */
#define CONFIG_SYS_NS16550_REG_SIZE	1

#define CONFIG_SYS_NS16550_CLK		0x03b9aca0
#define CONFIG_SYS_NS16550_COM1		0x18000300

#endif /* __ARCH_CONFIGS_H */
