/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2018 Philippe Reynes <philippe.reynes@softathome.com>
 */

#include <configs/bmips_common.h>
#include <configs/bmips_bcm6838.h>

#define CONFIG_ENV_SIZE			(8 * 1024)

#ifdef CONFIG_NAND
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_SELF_INIT
#define CONFIG_SYS_NAND_ONFI_DETECTION
#endif /* CONFIG_NAND */
