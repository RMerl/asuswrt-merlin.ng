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

#include "cpu_feature.h"

#if defined __i386__ || defined(__x86_64__)

typedef enum {
	/* Generic CPUID(1) flags */
	CPUID1_EDX_MMX =				(1 << 23),
	CPUID1_EDX_SSE =				(1 << 25),
	CPUID1_EDX_SSE2 =				(1 << 26),
	CPUID1_ECX_SSE3 =				(1 <<  0),
	CPUID1_ECX_PCLMULQDQ =			(1 <<  1),
	CPUID1_ECX_SSSE3 =				(1 <<  9),
	CPUID1_ECX_SSE41 =				(1 << 19),
	CPUID1_ECX_SSE42 =				(1 << 20),
	CPUID1_ECX_AESNI =				(1 << 25),
	CPUID1_ECX_AVX =				(1 << 28),
	CPUID1_ECX_RDRAND =				(1 << 30),

	/* For CentaurHauls cpuid(0xC0000001) */
	CPUIDC1_EDX_RNG_AVAILABLE =		(1 <<  2),
	CPUIDC1_EDX_RNG_ENABLED =		(1 <<  3),
	CPUIDC1_EDX_ACE_AVAILABLE =		(1 <<  6),
	CPUIDC1_EDX_ACE_ENABLED =		(1 <<  7),
	CPUIDC1_EDX_ACE2_AVAILABLE =	(1 <<  8),
	CPUIDC1_EDX_ACE2_ENABLED =		(1 <<  9),
	CPUIDC1_EDX_PHE_AVAILABLE =		(1 << 10),
	CPUIDC1_EDX_PHE_ENABLED =		(1 << 11),
	CPUIDC1_EDX_PMM_AVAILABLE =		(1 << 12),
	CPUIDC1_EDX_PMM_ENABLED =		(1 << 13),
} cpuid_flag_t;

/**
 * Get cpuid for info, return eax, ebx, ecx and edx.
 * -fPIC requires to save ebx on IA-32.
 */
static void cpuid(u_int op, u_int *a, u_int *b, u_int *c, u_int *d)
{
#ifdef __x86_64__
	asm("cpuid" : "=a" (*a), "=b" (*b), "=c" (*c), "=d" (*d) : "a" (op));
#else /* __i386__ */
	asm("pushl %%ebx;"
		"cpuid;"
		"movl %%ebx, %1;"
		"popl %%ebx;"
		: "=a" (*a), "=r" (*b), "=c" (*c), "=d" (*d) : "a" (op));
#endif /* __x86_64__ / __i386__*/
}

/**
 * Return feature if flag in reg, flag-to-feature
 */
static inline cpu_feature_t f2f(u_int reg, cpuid_flag_t flag, cpu_feature_t f)
{
	if (reg & flag)
	{
		return f;
	}
	return 0;
}

/**
 * Get features for a Via "CentaurHauls" CPU
 */
static cpu_feature_t get_via_features()
{
	cpu_feature_t f = 0;
	u_int a, b, c, d;

	cpuid(0xc0000001, &a, &b, &c, &d);

	f |= f2f(d, CPUIDC1_EDX_RNG_AVAILABLE, CPU_FEATURE_PADLOCK_RNG_AVAILABLE);
	f |= f2f(d, CPUIDC1_EDX_RNG_ENABLED, CPU_FEATURE_PADLOCK_RNG_ENABLED);
	f |= f2f(d, CPUIDC1_EDX_ACE_AVAILABLE, CPU_FEATURE_PADLOCK_ACE_AVAILABLE);
	f |= f2f(d, CPUIDC1_EDX_ACE_ENABLED, CPU_FEATURE_PADLOCK_ACE_ENABLED);
	f |= f2f(d, CPUIDC1_EDX_ACE2_AVAILABLE, CPU_FEATURE_PADLOCK_ACE2_AVAILABLE);
	f |= f2f(d, CPUIDC1_EDX_ACE2_ENABLED, CPU_FEATURE_PADLOCK_ACE2_ENABLED);
	f |= f2f(d, CPUIDC1_EDX_PHE_AVAILABLE, CPU_FEATURE_PADLOCK_PHE_AVAILABLE);
	f |= f2f(d, CPUIDC1_EDX_PHE_ENABLED, CPU_FEATURE_PADLOCK_PHE_ENABLED);
	f |= f2f(d, CPUIDC1_EDX_PMM_AVAILABLE, CPU_FEATURE_PADLOCK_PMM_AVAILABLE);
	f |= f2f(d, CPUIDC1_EDX_PMM_ENABLED, CPU_FEATURE_PADLOCK_PMM_ENABLED);

	return f;
}

/**
 * See header.
 */
cpu_feature_t cpu_feature_get_all()
{
	char vendor[3 * sizeof(uint32_t) + 1];
	cpu_feature_t f = 0;
	u_int a, b, c, d;

	cpuid(0, &a, &b, &c, &d);
	/* VendorID string is in b-d-c (yes, in this order) */
	snprintf(vendor, sizeof(vendor), "%.4s%.4s%.4s", &b, &d, &c);

	cpuid(1, &a, &b, &c, &d);

	/* check common x86 features for CPUID(1) */
	f |= f2f(d, CPUID1_EDX_MMX, CPU_FEATURE_MMX);
	f |= f2f(d, CPUID1_EDX_SSE, CPU_FEATURE_SSE);
	f |= f2f(d, CPUID1_EDX_SSE2, CPU_FEATURE_SSE2);
	f |= f2f(c, CPUID1_ECX_SSE3, CPU_FEATURE_SSE3);
	f |= f2f(c, CPUID1_ECX_PCLMULQDQ, CPU_FEATURE_PCLMULQDQ);
	f |= f2f(c, CPUID1_ECX_SSSE3, CPU_FEATURE_SSSE3);
	f |= f2f(c, CPUID1_ECX_SSE41, CPU_FEATURE_SSE41);
	f |= f2f(c, CPUID1_ECX_SSE42, CPU_FEATURE_SSE42);
	f |= f2f(c, CPUID1_ECX_AESNI, CPU_FEATURE_AESNI);
	f |= f2f(c, CPUID1_ECX_AVX, CPU_FEATURE_AVX);
	f |= f2f(c, CPUID1_ECX_RDRAND, CPU_FEATURE_RDRAND);

	if (streq(vendor, "CentaurHauls"))
	{
		cpuid(0xc0000000, &a, &b, &c, &d);
		/* check Centaur Extended Feature Flags */
		if (a >= 0xc0000001)
		{
			f |= get_via_features();
		}
	}
	return f;
}

#else /* !x86 */

/**
 * See header.
 */
cpu_feature_t cpu_feature_get_all()
{
	return 0;
}

#endif

/**
 * See header.
 */
bool cpu_feature_available(cpu_feature_t feature)
{
	return (cpu_feature_get_all() & feature) == feature;
}
