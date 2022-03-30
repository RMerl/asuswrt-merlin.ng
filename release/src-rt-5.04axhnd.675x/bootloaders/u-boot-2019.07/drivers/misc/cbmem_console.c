// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2011 The ChromiumOS Authors.  All rights reserved.
 */

#include <common.h>
#include <console.h>
#ifndef CONFIG_SYS_COREBOOT
#error This driver requires coreboot
#endif

#include <asm/arch/sysinfo.h>

struct cbmem_console {
	u32 buffer_size;
	u32 buffer_cursor;
	u8  buffer_body[0];
}  __attribute__ ((__packed__));

static struct cbmem_console *cbmem_console_p;

void cbmemc_putc(struct stdio_dev *dev, char data)
{
	int cursor;

	cursor = cbmem_console_p->buffer_cursor++;
	if (cursor < cbmem_console_p->buffer_size)
		cbmem_console_p->buffer_body[cursor] = data;
}

void cbmemc_puts(struct stdio_dev *dev, const char *str)
{
	char c;

	while ((c = *str++) != 0)
		cbmemc_putc(dev, c);
}

int cbmemc_init(void)
{
	int rc;
	struct stdio_dev cons_dev;
	cbmem_console_p = lib_sysinfo.cbmem_cons;

	memset(&cons_dev, 0, sizeof(cons_dev));

	strcpy(cons_dev.name, "cbmem");
	cons_dev.flags = DEV_FLAGS_OUTPUT; /* Output only */
	cons_dev.putc  = cbmemc_putc;
	cons_dev.puts  = cbmemc_puts;

	rc = stdio_register(&cons_dev);

	return (rc == 0) ? 1 : rc;
}
