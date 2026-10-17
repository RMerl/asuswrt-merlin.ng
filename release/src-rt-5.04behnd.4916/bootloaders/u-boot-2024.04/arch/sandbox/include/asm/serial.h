/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2020 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef __asm_serial_h
#define __asm_serial_h

#include <dt-structs.h>

struct sandbox_serial_plat {
#if CONFIG_IS_ENABLED(OF_PLATDATA)
	struct dtd_sandbox_serial dtplat;
#endif
	int colour;	/* Text colour to use for output, -1 for none */
};

/**
 * sandbox_serial_written() - Get the total number of characters written
 *
 * This returns the number of characters written by the sandbox serial
 * device. It is intended for performing tests of the serial subsystem
 * where a console buffer cannot be used. The absolute number should not be
 * relied upon; call this function twice and compare the difference.
 *
 * Return: The number of characters written
 */
size_t sandbox_serial_written(void);

/**
 * sandbox_serial_endisable() - Enable or disable serial output
 * @enabled: Whether serial output should be enabled or not
 *
 * This allows tests to enable or disable the sandbox serial output. All
 * processes relating to writing output (except the actual writing) will be
 * performed.
 */
void sandbox_serial_endisable(bool enabled);

/**
 * struct sandbox_serial_priv - Private data for this driver
 *
 * @buf: holds input characters available to be read by this driver
 */
struct sandbox_serial_priv {
	struct membuff buf;
	char serial_buf[16];
	bool start_of_line;
};

#endif /* __asm_serial_h */
