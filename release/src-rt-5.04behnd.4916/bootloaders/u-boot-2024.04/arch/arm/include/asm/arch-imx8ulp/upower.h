/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2021 NXP
 */

#ifndef __ASM_ARCH_IMX8ULP_UPOWER_H
#define __ASM_ARCH_IMX8ULP_UPOWER_H

#include <asm/types.h>

int upower_init(void);
int upower_pmic_i2c_write(u32 reg_addr, u32 reg_val);
int upower_pmic_i2c_read(u32 reg_addr, u32 *reg_val);

#endif
