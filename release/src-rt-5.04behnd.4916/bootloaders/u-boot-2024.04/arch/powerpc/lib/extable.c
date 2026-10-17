// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 1999  Magnus Damm <kieraypc01.p.y.kie.era.ericsson.se>
 *
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */
#include <common.h>
#include <log.h>

/*
 * The exception table consists of pairs of addresses: the first is the
 * address of an instruction that is allowed to fault, and the second is
 * the address at which the program should continue.  No registers are
 * modified, so it is entirely up to the continuation code to figure out
 * what to do.
 *
 * All the routines below use bits of fixup code that are out of line
 * with the main instruction path.  This means when everything is well,
 * we don't even have to jump over them.  Further, they do not intrude
 * on our cache or tlb entries.
 */

struct exception_table_entry
{
	unsigned long insn, fixup;
};

extern const struct exception_table_entry __start___ex_table[];
extern const struct exception_table_entry __stop___ex_table[];

static inline unsigned long
search_one_table(const struct exception_table_entry *first,
		 const struct exception_table_entry *last,
		 unsigned long value)
{
	long diff;
	while (first <= last) {
		diff = first->insn - value;
		if (diff == 0)
			return first->fixup;
		first++;
	}

	return 0;
}

unsigned long
search_exception_table(unsigned long addr)
{
	unsigned long ret;

	/* There is only the kernel to search.  */
	ret = search_one_table(__start___ex_table, __stop___ex_table-1, addr);
	/* if the serial port does not hang in exception, printf can be used */
#if !defined(CONFIG_SYS_SERIAL_HANG_IN_EXCEPTION)
	debug("Bus Fault @ 0x%08lx, fixup 0x%08lx\n", addr, ret);
#endif
	if (ret) return ret;

	return 0;
}
