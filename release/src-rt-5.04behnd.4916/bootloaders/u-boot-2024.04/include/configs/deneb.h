/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019 Siemens AG
 *
 */

#ifndef __DENEB_H
#define __DENEB_H

#include "capricorn-common.h"

/* DDR3 board total DDR is 2 GB */
#undef PHYS_SDRAM_1_SIZE
#define PHYS_SDRAM_1_SIZE		0x80000000	/* 2 GB */

#endif /* __DENEB_H */
