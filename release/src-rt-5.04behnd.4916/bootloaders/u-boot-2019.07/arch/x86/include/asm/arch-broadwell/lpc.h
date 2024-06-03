/* SPDX-License-Identifier: GPL-2.0 */
/*
 * From coreboot soc/intel/broadwell/include/soc/lpc.h
 *
 * Copyright (C) 2016 Google Inc.
 */

#ifndef _ASM_ARCH_LPC_H
#define _ASM_ARCH_LPC_H

#define GEN_PMCON_1		0xa0
#define  SMI_LOCK		(1 << 4)
#define GEN_PMCON_2		0xa2
#define  SYSTEM_RESET_STS	(1 << 4)
#define  THERMTRIP_STS		(1 << 3)
#define  SYSPWR_FLR		(1 << 1)
#define  PWROK_FLR		(1 << 0)
#define GEN_PMCON_3		0xa4
#define  SUS_PWR_FLR		(1 << 14)
#define  GEN_RST_STS		(1 << 9)
#define  RTC_BATTERY_DEAD	(1 << 2)
#define  PWR_FLR		(1 << 1)
#define  SLEEP_AFTER_POWER_FAIL	(1 << 0)
#define GEN_PMCON_LOCK		0xa6
#define  SLP_STR_POL_LOCK	(1 << 2)
#define  ACPI_BASE_LOCK		(1 << 1)
#define PMIR			0xac
#define  PMIR_CF9LOCK		(1 << 31)
#define  PMIR_CF9GR		(1 << 20)

#endif
