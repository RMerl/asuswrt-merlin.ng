/*
 * U-Boot - stub functions for common kgdb code,
 *          can be overridden in board specific files
 *
 * Copyright 2009 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <cpu_func.h>
#include <kgdb.h>
#include <serial.h>
#include <asm/ptrace.h>

int (*debugger_exception_handler)(struct pt_regs *);

__attribute__((weak))
void kgdb_serial_init(void)
{
	puts("[on serial] ");
}

__attribute__((weak))
void putDebugChar(int c)
{
	serial_putc(c);
}

__attribute__((weak))
void putDebugStr(const char *str)
{
#ifdef DEBUG
	serial_puts(str);
#endif
}

__attribute__((weak))
int getDebugChar(void)
{
	return serial_getc();
}

__attribute__((weak))
void kgdb_interruptible(int yes)
{
	return;
}

__attribute__((weak))
void kgdb_flush_cache_range(void *from, void *to)
{
	flush_cache((unsigned long)from, (unsigned long)(to - from));
}

__attribute__((weak))
void kgdb_flush_cache_all(void)
{
	if (dcache_status()) {
		dcache_disable();
		dcache_enable();
	}
	if (icache_status()) {
		icache_disable();
		icache_enable();
	}
}
