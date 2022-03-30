/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2011-2014 Panasonic Corporation
 */

#ifndef ARCH_ARM_MPCORE_H
#define ARCH_ARM_MPCORE_H

/* Snoop Control Unit */
#define SCU_OFFSET		0x00

/* SCU Control Register */
#define SCU_CTRL		0x00
#define SCU_ENABLE		(1 << 0)
#define SCU_STANDBY_ENABLE	(1 << 5)

/* SCU Configuration Register */
#define SCU_CONF		0x04
/* SCU CPU Power Status Register */
#define SCU_PWR_STATUS		0x08
/* SCU Invalidate All Registers in Secure State */
#define SCU_INV_ALL		0x0C
/* SCU Filtering Start Address Register */
#define SCU_FILTER_START	0x40
/* SCU Filtering End Address Register */
#define SCU_FILTER_END		0x44
/* SCU Access Control Register */
#define SCU_SAC			0x50
/* SCU Non-secure Access Control Register */
#define SCU_SNSAC		0x54

/* Global Timer */
#define GLOBAL_TIMER_OFFSET	0x200

/* Global Timer Counter Registers */
#define GTIMER_CNT_L		0x00
#define GTIMER_CNT_H		0x04
/* Global Timer Control Register */
#define GTIMER_CTRL		0x08
/* Global Timer Interrupt Status Register */
#define GTIMER_STAT		0x0C
/* Comparator Value Registers */
#define GTIMER_CMP_L		0x10
#define GTIMER_CMP_H		0x14
/* Auto-increment Register */
#define GTIMER_INC		0x18

#endif /* ARCH_ARM_MPCORE_H */
