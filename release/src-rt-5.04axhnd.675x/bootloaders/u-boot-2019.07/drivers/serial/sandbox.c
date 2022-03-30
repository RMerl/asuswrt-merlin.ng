// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2011 The Chromium OS Authors.
 */

/*
 * This provide a test serial port. It provides an emulated serial port where
 * a test program and read out the serial output and inject serial input for
 * U-Boot.
 */

#include <common.h>
#include <console.h>
#include <dm.h>
#include <fdtdec.h>
#include <lcd.h>
#include <os.h>
#include <serial.h>
#include <video.h>
#include <linux/compiler.h>
#include <asm/state.h>

DECLARE_GLOBAL_DATA_PTR;

#if CONFIG_IS_ENABLED(OF_CONTROL)

/*
 *
 *   serial_buf: A buffer that holds keyboard characters for the
 *		 Sandbox U-Boot.
 *
 * invariants:
 *   serial_buf_write		 == serial_buf_read -> empty buffer
 *   (serial_buf_write + 1) % 16 == serial_buf_read -> full buffer
 */
static char serial_buf[16];
static unsigned int serial_buf_write;
static unsigned int serial_buf_read;

struct sandbox_serial_platdata {
	int colour;	/* Text colour to use for output, -1 for none */
};

struct sandbox_serial_priv {
	bool start_of_line;
};

/**
 * output_ansi_colour() - Output an ANSI colour code
 *
 * @colour: Colour to output (0-7)
 */
static void output_ansi_colour(int colour)
{
	char ansi_code[] = "\x1b[1;3Xm";

	ansi_code[5] = '0' + colour;
	os_write(1, ansi_code, sizeof(ansi_code) - 1);
}

static void output_ansi_reset(void)
{
	os_write(1, "\x1b[0m", 4);
}

static int sandbox_serial_probe(struct udevice *dev)
{
	struct sandbox_state *state = state_get_current();
	struct sandbox_serial_priv *priv = dev_get_priv(dev);

	if (state->term_raw != STATE_TERM_COOKED)
		os_tty_raw(0, state->term_raw == STATE_TERM_RAW_WITH_SIGS);
	priv->start_of_line = 0;

	if (state->term_raw != STATE_TERM_RAW)
		disable_ctrlc(1);

	return 0;
}

static int sandbox_serial_remove(struct udevice *dev)
{
	struct sandbox_serial_platdata *plat = dev->platdata;

	if (plat->colour != -1)
		output_ansi_reset();

	return 0;
}

static int sandbox_serial_putc(struct udevice *dev, const char ch)
{
	struct sandbox_serial_priv *priv = dev_get_priv(dev);
	struct sandbox_serial_platdata *plat = dev->platdata;

	if (priv->start_of_line && plat->colour != -1) {
		priv->start_of_line = false;
		output_ansi_colour(plat->colour);
	}

	os_write(1, &ch, 1);
	if (ch == '\n')
		priv->start_of_line = true;

	return 0;
}

static unsigned int increment_buffer_index(unsigned int index)
{
	return (index + 1) % ARRAY_SIZE(serial_buf);
}

static int sandbox_serial_pending(struct udevice *dev, bool input)
{
	const unsigned int next_index =
		increment_buffer_index(serial_buf_write);
	ssize_t count;

	if (!input)
		return 0;

	os_usleep(100);
#ifndef CONFIG_SPL_BUILD
	video_sync_all();
#endif
	if (next_index == serial_buf_read)
		return 1;	/* buffer full */

	count = os_read(0, &serial_buf[serial_buf_write], 1);
	if (count == 1)
		serial_buf_write = next_index;

	return serial_buf_write != serial_buf_read;
}

static int sandbox_serial_getc(struct udevice *dev)
{
	int result;

	if (!sandbox_serial_pending(dev, true))
		return -EAGAIN;	/* buffer empty */

	result = serial_buf[serial_buf_read];
	serial_buf_read = increment_buffer_index(serial_buf_read);
	return result;
}
#endif /* CONFIG_IS_ENABLED(OF_CONTROL) */

#ifdef CONFIG_DEBUG_UART_SANDBOX

#include <debug_uart.h>

static inline void _debug_uart_init(void)
{
}

static inline void _debug_uart_putc(int ch)
{
	os_putc(ch);
}

DEBUG_UART_FUNCS

#endif /* CONFIG_DEBUG_UART_SANDBOX */

static int sandbox_serial_getconfig(struct udevice *dev, uint *serial_config)
{
	uint config = SERIAL_DEFAULT_CONFIG;

	if (!serial_config)
		return -EINVAL;

	*serial_config = config;

	return 0;
}

static int sandbox_serial_setconfig(struct udevice *dev, uint serial_config)
{
	u8 parity = SERIAL_GET_PARITY(serial_config);
	u8 bits = SERIAL_GET_BITS(serial_config);
	u8 stop = SERIAL_GET_STOP(serial_config);

	if (bits != SERIAL_8_BITS || stop != SERIAL_ONE_STOP ||
	    parity != SERIAL_PAR_NONE)
		return -ENOTSUPP; /* not supported in driver*/

	return 0;
}

static int sandbox_serial_getinfo(struct udevice *dev,
				  struct serial_device_info *serial_info)
{
	struct serial_device_info info = {
		.type = SERIAL_CHIP_UNKNOWN,
		.addr_space = SERIAL_ADDRESS_SPACE_IO,
		.addr = SERIAL_DEFAULT_ADDRESS,
		.reg_width = 1,
		.reg_offset = 0,
		.reg_shift = 0,
	};

	if (!serial_info)
		return -EINVAL;

	*serial_info = info;

	return 0;
}

#if CONFIG_IS_ENABLED(OF_CONTROL)
static const char * const ansi_colour[] = {
	"black", "red", "green", "yellow", "blue", "megenta", "cyan",
	"white",
};

static int sandbox_serial_ofdata_to_platdata(struct udevice *dev)
{
	struct sandbox_serial_platdata *plat = dev->platdata;
	const char *colour;
	int i;

	plat->colour = -1;
	colour = fdt_getprop(gd->fdt_blob, dev_of_offset(dev),
			     "sandbox,text-colour", NULL);
	if (colour) {
		for (i = 0; i < ARRAY_SIZE(ansi_colour); i++) {
			if (!strcmp(colour, ansi_colour[i])) {
				plat->colour = i;
				break;
			}
		}
	}

	return 0;
}

static const struct dm_serial_ops sandbox_serial_ops = {
	.putc = sandbox_serial_putc,
	.pending = sandbox_serial_pending,
	.getc = sandbox_serial_getc,
	.getconfig = sandbox_serial_getconfig,
	.setconfig = sandbox_serial_setconfig,
	.getinfo = sandbox_serial_getinfo,
};

static const struct udevice_id sandbox_serial_ids[] = {
	{ .compatible = "sandbox,serial" },
	{ }
};

U_BOOT_DRIVER(serial_sandbox) = {
	.name	= "serial_sandbox",
	.id	= UCLASS_SERIAL,
	.of_match = sandbox_serial_ids,
	.ofdata_to_platdata = sandbox_serial_ofdata_to_platdata,
	.platdata_auto_alloc_size = sizeof(struct sandbox_serial_platdata),
	.priv_auto_alloc_size = sizeof(struct sandbox_serial_priv),
	.probe = sandbox_serial_probe,
	.remove = sandbox_serial_remove,
	.ops	= &sandbox_serial_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

static const struct sandbox_serial_platdata platdata_non_fdt = {
	.colour = -1,
};

U_BOOT_DEVICE(serial_sandbox_non_fdt) = {
	.name = "serial_sandbox",
	.platdata = &platdata_non_fdt,
};
#endif /* CONFIG_IS_ENABLED(OF_CONTROL) */
