// SPDX-License-Identifier: GPL-2.0+
/*
 * PXA CPU information display
 *
 * Copyright (C) 2011 Marek Vasut <marek.vasut@gmail.com>
 */

#include <common.h>
#include <asm/io.h>
#include <errno.h>
#include <linux/compiler.h>

#ifdef CONFIG_CPU_PXA25X
#if ((CONFIG_SYS_INIT_SP_ADDR) != 0xfffff800)
#error "Init SP address must be set to 0xfffff800 for PXA250"
#endif
#endif

#define	CPU_MASK_PXA_PRODID	0x000003f0
#define	CPU_MASK_PXA_REVID	0x0000000f

#define	CPU_MASK_PRODREV	(CPU_MASK_PXA_PRODID | CPU_MASK_PXA_REVID)

#define	CPU_VALUE_PXA25X	0x100
#define	CPU_VALUE_PXA27X	0x110

static uint32_t pxa_get_cpuid(void)
{
	uint32_t cpuid;
	asm volatile("mrc p15, 0, %0, c0, c0, 0" : "=r"(cpuid));
	return cpuid;
}

int cpu_is_pxa25x(void)
{
	uint32_t id = pxa_get_cpuid();
	id &= CPU_MASK_PXA_PRODID;
	return id == CPU_VALUE_PXA25X;
}

int cpu_is_pxa27x(void)
{
	uint32_t id = pxa_get_cpuid();
	id &= CPU_MASK_PXA_PRODID;
	return id == CPU_VALUE_PXA27X;
}

int cpu_is_pxa27xm(void)
{
	uint32_t id = pxa_get_cpuid();
	return ((id & CPU_MASK_PXA_PRODID) == CPU_VALUE_PXA27X) &&
			((id & CPU_MASK_PXA_REVID) == 8);
}

uint32_t pxa_get_cpu_revision(void)
{
	return pxa_get_cpuid() & CPU_MASK_PRODREV;
}

#ifdef	CONFIG_DISPLAY_CPUINFO
static const char *pxa25x_get_revision(void)
{
	static __maybe_unused const char * const revs_25x[] = { "A0" };
	static __maybe_unused const char * const revs_26x[] = {
								"A0", "B0", "B1"
								};
	static const char *unknown = "Unknown";
	uint32_t id;

	if (!cpu_is_pxa25x())
		return unknown;

	id = pxa_get_cpuid() & CPU_MASK_PXA_REVID;

/* PXA26x is a sick special case as it can't be told apart from PXA25x :-( */
#ifdef	CONFIG_CPU_PXA26X
	switch (id) {
	case 3: return revs_26x[0];
	case 5: return revs_26x[1];
	case 6: return revs_26x[2];
	}
#else
	if (id == 6)
		return revs_25x[0];
#endif
	return unknown;
}

static const char *pxa27x_get_revision(void)
{
	static const char *const rev[] = { "A0", "A1", "B0", "B1", "C0", "C5" };
	static const char *unknown = "Unknown";
	uint32_t id;

	if (!cpu_is_pxa27x())
		return unknown;

	id = pxa_get_cpuid() & CPU_MASK_PXA_REVID;

	if ((id == 5) || (id == 6) || (id > 8))
		return unknown;

	/* Cap the special PXA270 C5 case. */
	if (id == 7)
		id = 5;

	/* Cap the special PXA270M A1 case. */
	if (id == 8)
		id = 1;

	return rev[id];
}

static int print_cpuinfo_pxa2xx(void)
{
	if (cpu_is_pxa25x()) {
		puts("Marvell PXA25x rev. ");
		puts(pxa25x_get_revision());
	} else if (cpu_is_pxa27x()) {
		puts("Marvell PXA27x");
		if (cpu_is_pxa27xm()) puts("M");
		puts(" rev. ");
		puts(pxa27x_get_revision());
	} else
		return -EINVAL;

	puts("\n");

	return 0;
}

int print_cpuinfo(void)
{
	int ret;

	puts("CPU: ");

	ret = print_cpuinfo_pxa2xx();
	if (!ret)
		return ret;

	return ret;
}
#endif
