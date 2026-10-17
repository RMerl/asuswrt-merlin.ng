// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2011 The ChromiumOS Authors.  All rights reserved.
 */

#include <common.h>
#include <console.h>
#include <asm/cb_sysinfo.h>

void cbmemc_putc(struct stdio_dev *dev, char data)
{
	const struct sysinfo_t *sysinfo = cb_get_sysinfo();
	struct cbmem_console *cons;
	uint pos, flags;

	if (!sysinfo)
		return;
	cons = sysinfo->cbmem_cons;
	if (!cons)
		return;

	pos = cons->cursor & CBMC_CURSOR_MASK;

	/* preserve the overflow flag if present */
	flags = cons->cursor & ~CBMC_CURSOR_MASK;

	cons->body[pos++] = data;

	/*
	 * Deal with overflow - the flag may be cleared by another program which
	 * reads the buffer out later, e.g. Linux
	 */
	if (pos >= cons->size) {
		pos = 0;
		flags |= CBMC_OVERFLOW;
	}

	cons->cursor = flags | pos;
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

	memset(&cons_dev, 0, sizeof(cons_dev));

	strcpy(cons_dev.name, "cbmem");
	cons_dev.flags = DEV_FLAGS_OUTPUT; /* Output only */
	cons_dev.putc  = cbmemc_putc;
	cons_dev.puts  = cbmemc_puts;

	rc = stdio_register(&cons_dev);

	return (rc == 0) ? 1 : rc;
}
