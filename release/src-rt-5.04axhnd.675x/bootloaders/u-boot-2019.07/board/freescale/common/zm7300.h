/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2013 Freescale Semiconductor, Inc.
 */

#ifndef __ZM7300_H_
#define __ZM7300_H	1_

#include <common.h>
#include <i2c.h>
#include <errno.h>
#include <asm/io.h>

#define ZM_STEP 125
int zm7300_set_voltage(int voltage_1_10mv);
int zm_write_voltage(int voltage);
int zm_read_voltage(void);
int zm_disable_wp(void);
int zm_enable_wp(void);

#endif	/* __ZM7300_H_ */
