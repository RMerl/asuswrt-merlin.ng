/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian@popies.net>
 * Lead Tech Design <www.leadtechdesign.com>
 */
#ifndef __ASM_ARM_ARCH_HARDWARE_H__
#define __ASM_ARM_ARCH_HARDWARE_H__

#if defined(CONFIG_AT91RM9200)
# include <asm/arch/at91rm9200.h>
#elif defined(CONFIG_AT91SAM9260) || defined(CONFIG_AT91SAM9G20) || \
	defined(CONFIG_AT91SAM9XE)
# include <asm/arch/at91sam9260.h>
#elif defined(CONFIG_AT91SAM9261) || defined(CONFIG_AT91SAM9G10)
# include <asm/arch/at91sam9261.h>
#elif defined(CONFIG_AT91SAM9263)
# include <asm/arch/at91sam9263.h>
#elif defined(CONFIG_AT91SAM9RL)
# include <asm/arch/at91sam9rl.h>
#elif defined(CONFIG_AT91SAM9G45) || defined(CONFIG_AT91SAM9M10G45)
# include <asm/arch/at91sam9g45.h>
#elif defined(CONFIG_AT91SAM9N12) || defined(CONFIG_AT91SAM9X5)
# include <asm/arch/at91sam9x5.h>
#elif defined(CONFIG_SAMA5D2)
# include <asm/arch/sama5d2.h>
#elif defined(CONFIG_SAMA5D3)
# include <asm/arch/sama5d3.h>
#elif defined(CONFIG_SAMA5D4)
# include <asm/arch/sama5d4.h>
#else
# error "Unsupported AT91 processor"
#endif

#endif /* __ASM_ARM_ARCH_HARDWARE_H__ */
