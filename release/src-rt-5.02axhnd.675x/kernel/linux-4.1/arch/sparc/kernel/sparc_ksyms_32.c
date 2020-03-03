/*
 * arch/sparc/kernel/ksyms.c: Sparc specific ksyms support.
 *
 * Copyright (C) 1996 David S. Miller (davem@caip.rutgers.edu)
 * Copyright (C) 1996 Eddie C. Dost (ecd@skynet.be)
 */

#include <linux/module.h>

#include <asm/pgtable.h>
#include <asm/uaccess.h>
#include <asm/delay.h>
#include <asm/head.h>
#include <asm/dma.h>

struct poll {
	int fd;
	short events;
	short revents;
};

/* from entry.S */
EXPORT_SYMBOL(__udelay);
EXPORT_SYMBOL(__ndelay);

/* from head_32.S */
EXPORT_SYMBOL(__ret_efault);
EXPORT_SYMBOL(empty_zero_page);

/* Exporting a symbol from /init/main.c */
EXPORT_SYMBOL(saved_command_line);
