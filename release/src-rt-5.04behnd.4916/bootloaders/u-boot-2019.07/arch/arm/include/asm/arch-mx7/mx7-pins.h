/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015 Freescale Semiconductor, Inc.
 */
#ifndef __ASM_ARCH_MX7_PINS_H__
#define __ASM_ARCH_MX7_PINS_H__

#include <asm/mach-imx/iomux-v3.h>

#if defined(CONFIG_MX7D)
#include "mx7d_pins.h"
#elif defined(CONFIG_MX7S)
#include "mx7s_pins.h"
#else
#error "Please select cpu"
#endif	/* CONFIG_MX7D */

#endif	/*__ASM_ARCH_MX7_PINS_H__ */
