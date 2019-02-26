/*
 * Copyright (C) 2015 Martin Willi
 * Copyright (C) 2015 revosec AG
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

/**
 * @defgroup cpu_feature cpu_feature
 * @{ @ingroup utils
 */

#ifndef CPU_FEATURE_H_
#define CPU_FEATURE_H_

#include <library.h>

typedef enum {
	/** x86/x64 extensions */
	CPU_FEATURE_MMX =							(1 <<  0),
	CPU_FEATURE_SSE =							(1 <<  1),
	CPU_FEATURE_SSE2 =							(1 <<  2),
	CPU_FEATURE_SSE3 =							(1 <<  3),
	CPU_FEATURE_SSSE3 =							(1 <<  4),
	CPU_FEATURE_SSE41 =							(1 <<  5),
	CPU_FEATURE_SSE42 =							(1 <<  6),
	CPU_FEATURE_AVX =							(1 <<  7),
	CPU_FEATURE_RDRAND =						(1 <<  8),
	CPU_FEATURE_AESNI =							(1 <<  9),
	CPU_FEATURE_PCLMULQDQ =						(1 << 10),
	/** Via Padlock Security features */
	CPU_FEATURE_PADLOCK_RNG_AVAILABLE =			(1 << 22),
	CPU_FEATURE_PADLOCK_RNG_ENABLED =			(1 << 23),
	CPU_FEATURE_PADLOCK_ACE_AVAILABLE =			(1 << 24),
	CPU_FEATURE_PADLOCK_ACE_ENABLED =			(1 << 25),
	CPU_FEATURE_PADLOCK_ACE2_AVAILABLE =		(1 << 26),
	CPU_FEATURE_PADLOCK_ACE2_ENABLED =			(1 << 27),
	CPU_FEATURE_PADLOCK_PHE_AVAILABLE =			(1 << 28),
	CPU_FEATURE_PADLOCK_PHE_ENABLED =			(1 << 29),
	CPU_FEATURE_PADLOCK_PMM_AVAILABLE =			(1 << 30),
	CPU_FEATURE_PADLOCK_PMM_ENABLED =			(1 << 31),
} cpu_feature_t;

/**
 * Get a bitmask for all supported CPU features
 */
cpu_feature_t cpu_feature_get_all();

/**
 * Check if a given set of CPU features is available.
 */
bool cpu_feature_available(cpu_feature_t feature);

#endif /** CPU_FEATURE_H_ @}*/
