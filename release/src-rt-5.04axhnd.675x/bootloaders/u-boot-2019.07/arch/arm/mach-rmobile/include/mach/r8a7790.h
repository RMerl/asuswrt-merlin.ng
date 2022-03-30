/* SPDX-License-Identifier: GPL-2.0 */
/*
 * arch/arm/include/asm/arch-rmobile/r8a7790.h
 *
 * Copyright (C) 2013,2014 Renesas Electronics Corporation
 */

#ifndef __ASM_ARCH_R8A7790_H
#define __ASM_ARCH_R8A7790_H

#include "rcar-base.h"

/* SH-I2C */
#define CONFIG_SYS_I2C_SH_BASE2	0xE6520000
#define CONFIG_SYS_I2C_SH_BASE3	0xE60B0000

/* Module stop control/status register bits */
#define MSTP0_BITS	0x00640801
#define MSTP1_BITS	0xDB6E9BDF
#define MSTP2_BITS	0x300DA1FC
#define MSTP3_BITS	0xF08CF831
#define MSTP4_BITS	0x80000184
#define MSTP5_BITS	0x44C00046
#define MSTP7_BITS	0x07F30718
#define MSTP8_BITS	0x01F0FF84
#define MSTP9_BITS	0xF5979FCF
#define MSTP10_BITS	0xFFFEFFE0
#define MSTP11_BITS	0x00000000

/* SDHI */
#define CONFIG_SYS_SH_SDHI1_BASE 0xEE120000
#define CONFIG_SYS_SH_SDHI2_BASE 0xEE140000
#define CONFIG_SYS_SH_SDHI3_BASE 0xEE160000
#define CONFIG_SYS_SH_SDHI_NR_CHANNEL 4

#define R8A7790_CUT_ES2X	2
#define IS_R8A7790_ES2()	\
	(rmobile_get_cpu_rev_integer() == R8A7790_CUT_ES2X)

#endif /* __ASM_ARCH_R8A7790_H */
