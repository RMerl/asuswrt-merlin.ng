/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2011 Simon Guinot <sguinot@lacie.com>
 *
 * Based on Kirkwood support:
 * (C) Copyright 2009
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
 */

#ifndef NET2BIG_V2_H
#define NET2BIG_V2_H

/* GPIO configuration */
#define NET2BIG_V2_OE_LOW		0x0600E000
#define NET2BIG_V2_OE_HIGH		0x00000134
#define NET2BIG_V2_OE_VAL_LOW		0x10030000
#define NET2BIG_V2_OE_VAL_HIGH		0x00000000

/* Buttons */
#define NET2BIG_V2_GPIO_PUSH_BUTTON	34

/* GMT G762 registers (I2C fan controller) */
#define G762_REG_SET_CNT		0x00
#define G762_REG_SET_OUT		0x03
#define G762_REG_FAN_CMD1		0x04

#endif /* NET2BIG_V2_H */
