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

/* Set the board specific parameters */
#define DEF_ENV_TFTPDIR		"theadorable-x86-conga"
#define DEF_ENV_ETH_INIT	""
#define DEF_ENV_UBUNTU_PART	2
#define DEF_ENV_UBUNTU_TTY	0	/* Use ttyS0 */
#define DEF_ENV_YOCTO_PART	3
#define DEF_ENV_YOCTO_TTY	0	/* Use ttyS0 */

/*
 * Include the theadorable-x86 common options, macros and default
 * environment
 */
#include <configs/theadorable-x86-common.h>

#endif	/* __CONFIG_H */
