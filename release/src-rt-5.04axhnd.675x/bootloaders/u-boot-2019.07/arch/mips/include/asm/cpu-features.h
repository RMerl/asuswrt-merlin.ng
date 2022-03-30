/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2003, 2004 Ralf Baechle
 * Copyright (C) 2004  Maciej W. Rozycki
 */
#ifndef __ASM_CPU_FEATURES_H
#define __ASM_CPU_FEATURES_H

#include <cpu-feature-overrides.h>

#ifdef CONFIG_32BIT
# ifndef cpu_has_64bits
# define cpu_has_64bits			0
# endif
# ifndef cpu_has_64bit_addresses
# define cpu_has_64bit_addresses	0
# endif
#endif

#ifdef CONFIG_64BIT
# ifndef cpu_has_64bits
# define cpu_has_64bits			1
# endif
# ifndef cpu_has_64bit_addresses
# define cpu_has_64bit_addresses	1
# endif
#endif

#endif /* __ASM_CPU_FEATURES_H */
