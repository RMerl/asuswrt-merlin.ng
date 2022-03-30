// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Microsemi Jaguar2 Switch driver
 *
 * Copyright (c) 2018 Microsemi Corporation
 */

#ifndef _MSCC_JR2_H_
#define _MSCC_JR2_H_

#include <linux/bitops.h>
#include <dm.h>

/*
 * Target offset base(s)
 */
#define MSCC_IO_ORIGIN1_OFFSET 0x70000000
#define MSCC_IO_ORIGIN1_SIZE   0x00200000
#define MSCC_IO_ORIGIN2_OFFSET 0x71000000
#define MSCC_IO_ORIGIN2_SIZE   0x01000000
#define BASE_CFG        ((void __iomem *)0x70000000)
#define BASE_DEVCPU_GCB ((void __iomem *)0x71010000)

#endif
