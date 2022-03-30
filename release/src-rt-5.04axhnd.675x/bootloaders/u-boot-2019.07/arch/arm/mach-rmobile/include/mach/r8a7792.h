/* SPDX-License-Identifier: GPL-2.0 */
/*
 * arch/arm/include/asm/arch-rmobile/r8a7792.h
 *
 * Copyright (C) 2016 Renesas Electronics Corporation
 */

#ifndef __ASM_ARCH_R8A7792_H
#define __ASM_ARCH_R8A7792_H

#include "rcar-base.h"

/* SH-I2C */
#define CONFIG_SYS_I2C_SH_BASE2	0xE6520000
#define CONFIG_SYS_I2C_SH_BASE3	0xE60B0000

/* Module stop control/status register bits */
#define MSTP0_BITS	0x00400801
#define MSTP1_BITS	0x9B6F987F
#define MSTP2_BITS	0x108CE100
#define MSTP3_BITS	0x20004010
#define MSTP4_BITS	0x80000184
#define MSTP5_BITS	0x44C00004
#define MSTP7_BITS	0x01BF0000
#define MSTP8_BITS	0x1FE01FB0
#define MSTP9_BITS	0xFE2BFFB2
#define MSTP10_BITS	0x00001820
#define MSTP11_BITS	0x00000008

/* SDHI */
#define CONFIG_SYS_SH_SDHI_NR_CHANNEL 1

#endif /* __ASM_ARCH_R8A7792_H */
