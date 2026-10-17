/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2009 Jean-Christophe PLAGNIOL-VILLARD <plagnioj@jrosoft.com>
 */

#ifndef __ASM_ARCH_AT91SAM9_MATRIX_H
#define __ASM_ARCH_AT91SAM9_MATRIX_H

#if defined(CONFIG_AT91SAM9260) || defined(CONFIG_AT91SAM9G20)
#include <asm/arch/at91sam9260_matrix.h>
#elif defined(CONFIG_AT91SAM9261)
#include <asm/arch/at91sam9261_matrix.h>
#elif defined(CONFIG_AT91SAM9263)
#include <asm/arch/at91sam9263_matrix.h>
#elif defined(CONFIG_AT91SAM9RL)
#include <asm/arch/at91sam9rl_matrix.h>
#elif defined(CONFIG_AT91SAM9G45) || defined(CONFIG_AT91SAM9M10G45)
#include <asm/arch/at91sam9g45_matrix.h>
#elif defined(CONFIG_AT91SAM9N12) || defined(CONFIG_AT91SAM9X5)
#include <asm/arch/at91sam9x5_matrix.h>
#else
#error "Unsupported AT91SAM9/CAP9 processor"
#endif

#endif /* __ASM_ARCH_AT91SAM9_MATRIX_H */
