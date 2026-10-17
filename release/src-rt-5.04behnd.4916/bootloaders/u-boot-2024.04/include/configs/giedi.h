/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019 Siemens AG
 *
 */

#ifndef __GIEDI_H
#define __GIEDI_H

#include "capricorn-common.h"

/* DDR3 board total DDR is 1 GB */
#undef PHYS_SDRAM_1_SIZE
#define PHYS_SDRAM_1_SIZE		0x40000000	/* 1 GB */

#endif /* __GIEDI_H */
