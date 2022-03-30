// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013 Imagination Technologies
 * Author: Paul Burton <paul.burton@mips.com>
 *
 * Setup code for the FDC37M817 super I/O controller
 */

#include <common.h>
#include <asm/io.h>

#define SIO_CONF_PORT		0x3f0
#define SIO_DATA_PORT		0x3f1

enum sio_conf_key {
	SIOCONF_DEVNUM		= 0x07,
	SIOCONF_ACTIVATE	= 0x30,
	SIOCONF_ENTER_SETUP	= 0x55,
	SIOCONF_BASE_HIGH	= 0x60,
	SIOCONF_BASE_LOW	= 0x61,
	SIOCONF_PRIMARY_INT	= 0x70,
	SIOCONF_EXIT_SETUP	= 0xaa,
	SIOCONF_MODE		= 0xf0,
};

static struct {
	u8 key;
	u8 data;
} sio_config[] = {
	/* tty0 */
	{ SIOCONF_DEVNUM,	0x04 },
	{ SIOCONF_BASE_HIGH,	0x03 },
	{ SIOCONF_BASE_LOW,	0xf8 },
	{ SIOCONF_MODE,		0x02 },
	{ SIOCONF_PRIMARY_INT,	0x04 },
	{ SIOCONF_ACTIVATE,	0x01 },

	/* tty1 */
	{ SIOCONF_DEVNUM,	0x05 },
	{ SIOCONF_BASE_HIGH,	0x02 },
	{ SIOCONF_BASE_LOW,	0xf8 },
	{ SIOCONF_MODE,		0x02 },
	{ SIOCONF_PRIMARY_INT,	0x03 },
	{ SIOCONF_ACTIVATE,	0x01 },
};

void malta_superio_init(void)
{
	unsigned i;

	/* enter config state */
	outb(SIOCONF_ENTER_SETUP, SIO_CONF_PORT);

	/* configure peripherals */
	for (i = 0; i < ARRAY_SIZE(sio_config); i++) {
		outb(sio_config[i].key, SIO_CONF_PORT);
		outb(sio_config[i].data, SIO_DATA_PORT);
	}

	/* exit config state */
	outb(SIOCONF_EXIT_SETUP, SIO_CONF_PORT);
}
