// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Thomas Chou <thomas@wytron.com.tw>
 * Copyright (C) 2009, Wind River Systems Inc
 * Implemented by fredrik.markstrom@gmail.com and ivarholmqvist@gmail.com
 */

#include <common.h>
#include <cpu_func.h>
#include <asm/cache.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

static void __flush_dcache(unsigned long start, unsigned long end)
{
	unsigned long addr;

	start &= ~(gd->arch.dcache_line_size - 1);
	end += (gd->arch.dcache_line_size - 1);
	end &= ~(gd->arch.dcache_line_size - 1);

	for (addr = start; addr < end; addr += gd->arch.dcache_line_size) {
		__asm__ __volatile__ ("   flushda 0(%0)\n"
					: /* Outputs */
					: /* Inputs  */ "r"(addr)
					/* : No clobber */);
	}
}

static void __flush_dcache_all(unsigned long start, unsigned long end)
{
	unsigned long addr;

	start &= ~(gd->arch.dcache_line_size - 1);
	end += (gd->arch.dcache_line_size - 1);
	end &= ~(gd->arch.dcache_line_size - 1);

	if (end > start + gd->arch.dcache_size)
		end = start + gd->arch.dcache_size;

	for (addr = start; addr < end; addr += gd->arch.dcache_line_size) {
		__asm__ __volatile__ ("   flushd 0(%0)\n"
					: /* Outputs */
					: /* Inputs  */ "r"(addr)
					/* : No clobber */);
	}
}

static void __invalidate_dcache(unsigned long start, unsigned long end)
{
	unsigned long addr;

	start &= ~(gd->arch.dcache_line_size - 1);
	end += (gd->arch.dcache_line_size - 1);
	end &= ~(gd->arch.dcache_line_size - 1);

	for (addr = start; addr < end; addr += gd->arch.dcache_line_size) {
		__asm__ __volatile__ ("   initda 0(%0)\n"
					: /* Outputs */
					: /* Inputs  */ "r"(addr)
					/* : No clobber */);
	}
}

static void __flush_icache(unsigned long start, unsigned long end)
{
	unsigned long addr;

	start &= ~(gd->arch.icache_line_size - 1);
	end += (gd->arch.icache_line_size - 1);
	end &= ~(gd->arch.icache_line_size - 1);

	if (end > start + gd->arch.icache_size)
		end = start + gd->arch.icache_size;

	for (addr = start; addr < end; addr += gd->arch.icache_line_size) {
		__asm__ __volatile__ ("   flushi %0\n"
					: /* Outputs */
					: /* Inputs  */ "r"(addr)
					/* : No clobber */);
	}
	__asm__ __volatile(" flushp\n");
}

void flush_dcache_all(void)
{
	__flush_dcache_all(0, gd->arch.dcache_size);
	__flush_icache(0, gd->arch.icache_size);
}

void flush_dcache_range(unsigned long start, unsigned long end)
{
	if (gd->arch.has_initda)
		__flush_dcache(start, end);
	else
		__flush_dcache_all(start, end);
}

void flush_cache(unsigned long start, unsigned long size)
{
	if (gd->arch.has_initda)
		__flush_dcache(start, start + size);
	else
		__flush_dcache_all(start, start + size);
	__flush_icache(start, start + size);
}

void invalidate_dcache_range(unsigned long start, unsigned long end)
{
	if (gd->arch.has_initda)
		__invalidate_dcache(start, end);
	else
		__flush_dcache_all(start, end);
}

int dcache_status(void)
{
	return 1;
}

void dcache_enable(void)
{
	flush_dcache_all();
}

void dcache_disable(void)
{
	flush_dcache_all();
}
