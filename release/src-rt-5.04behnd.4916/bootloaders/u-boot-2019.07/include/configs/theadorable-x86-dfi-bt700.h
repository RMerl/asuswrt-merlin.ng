/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016 Stefan Roese <sr@denx.de>
 */

/*
 * board/config.h - configuration options, board specific
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <configs/x86-common.h>

/* Use BayTrail internal HS UART which is memory-mapped */
#undef  CONFIG_SYS_NS16550_PORT_MAPPED

/* Set the board specific parameters */
#define DEF_ENV_TFTPDIR		"theadorable-x86-dfi"
#define DEF_ENV_ETH_INIT	"usb reset"
#define DEF_ENV_UBUNTU_PART	1
#define DEF_ENV_UBUNTU_TTY	4	/* Use ttyS4 */
#define DEF_ENV_YOCTO_PART	2
#define DEF_ENV_YOCTO_TTY	1	/* Use ttyS1 */

/*
 * Include the theadorable-x86 common options, macros and default
 * environment
 */
#include <configs/theadorable-x86-common.h>

#endif	/* __CONFIG_H */
