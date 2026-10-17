/*
 * Simulate a SPI port and clients (see doc/arch/sandbox.rst for details)
 *
 * Copyright (c) 2011-2013 The Chromium OS Authors.
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * Licensed under the GPL-2 or later.
 */

#ifndef __ASM_SPI_H__
#define __ASM_SPI_H__

#include <linux/types.h>

/*
 * The interface between the SPI bus and the SPI client.  The bus will
 * instantiate a client, and that then call into it via these entry
 * points.  These should be enough for the client to emulate the SPI
 * device just like the real hardware.
 */
struct sandbox_spi_emu_ops {
	/* The bus wants to instantiate a new client, so setup everything */
	int (*setup)(void **priv, const char *spec);
	/* The bus is done with us, so break things down */
	void (*free)(void *priv);
	/* The CS has been "activated" -- we won't worry about low/high */
	void (*cs_activate)(void *priv);
	/* The CS has been "deactivated" -- we won't worry about low/high */
	void (*cs_deactivate)(void *priv);
	/* The client is rx-ing bytes from the bus, so it should tx some */
	int (*xfer)(void *priv, const u8 *rx, u8 *tx, uint bytes);
};

#endif
