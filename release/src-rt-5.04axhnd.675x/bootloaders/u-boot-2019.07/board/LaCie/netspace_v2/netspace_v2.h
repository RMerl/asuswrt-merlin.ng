/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2011 Simon Guinot <sguinot@lacie.com>
 *
 * Based on Kirkwood support:
 * (C) Copyright 2009
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
 */

#ifndef NETSPACE_V2_H
#define NETSPACE_V2_H

/* GPIO configuration */
#define NETSPACE_V2_OE_LOW		0x06004000
#define NETSPACE_V2_OE_HIGH		0x00000031
#define NETSPACE_V2_OE_VAL_LOW		0x10030000
#define NETSPACE_V2_OE_VAL_HIGH		0x00000000

#define NETSPACE_V2_GPIO_BUTTON         32

#endif /* NETSPACE_V2_H */
