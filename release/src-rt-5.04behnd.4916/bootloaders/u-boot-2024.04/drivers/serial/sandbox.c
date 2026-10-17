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
#include <os.h>
#include <serial.h>
#include <video.h>
#include <asm/global_data.h>
#include <linux/compiler.h>
#include <asm/serial.h>
#include <asm/state.h>

DECLARE_GLOBAL_DATA_PTR;

static size_t _sandbox_serial_written = 1;
static bool sandbox_serial_enabled = true;

size_t sandbox_serial_written(void)
{
	return _sandbox_serial_written;
}

void sandbox_serial_endisable(bool enabled)
{
	sandbox_serial_enabled = enabled;
}

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
	membuff_init(&priv->buf, priv->serial_buf, sizeof(priv->serial_buf));

	return 0;
}

static int sandbox_serial_remove(struct udevice *dev)
{
	struct sandbox_serial_plat *plat = dev_get_plat(dev);

	if (plat->colour != -1)
		output_ansi_reset();

	return 0;
}

static void sandbox_print_color(struct udevice *dev)
{
	struct sandbox_serial_priv *priv = dev_get_priv(dev);
	struct sandbox_serial_plat *plat = dev_get_plat(dev);

	/* With of-platdata we don't real the colour correctly, so disable it */
	if (!CONFIG_IS_ENABLED(OF_PLATDATA) && priv->start_of_line &&
	    plat->colour != -1) {
		priv->start_of_line = false;
		output_ansi_colour(plat->colour);
	}
}

static int sandbox_serial_putc(struct udevice *dev, const char ch)
{
	struct sandbox_serial_priv *priv = dev_get_priv(dev);

	if (ch == '\n')
		priv->start_of_line = true;

	if (sandbox_serial_enabled) {
		sandbox_print_color(dev);
		os_write(1, &ch, 1);
	}
	_sandbox_serial_written += 1;
	return 0;
}

static ssize_t sandbox_serial_puts(struct udevice *dev, const char *s,
				   size_t len)
{
	struct sandbox_serial_priv *priv = dev_get_priv(dev);
	ssize_t ret;

	if (len && s[len - 1] == '\n')
		priv->start_of_line = true;

	if (sandbox_serial_enabled) {
		sandbox_print_color(dev);
		ret = os_write(1, s, len);
		if (ret < 0)
			return ret;
	} else {
		ret = len;
	}
	_sandbox_serial_written += ret;
	return ret;
}

static int sandbox_serial_pending(struct udevice *dev, bool input)
{
	struct sandbox_serial_priv *priv = dev_get_priv(dev);
	ssize_t count;
	char *data;
	int avail;

	if (!input)
		return 0;

	os_usleep(100);
	if (IS_ENABLED(CONFIG_VIDEO) && !IS_ENABLED(CONFIG_SPL_BUILD))
		video_sync_all();
	avail = membuff_putraw(&priv->buf, 100, false, &data);
	if (!avail)
		return 1;	/* buffer full */

	count = os_read(0, data, avail);
	if (count > 0)
		membuff_putraw(&priv->buf, count, true, &data);

	return membuff_avail(&priv->buf);
}

static int sandbox_serial_getc(struct udevice *dev)
{
	struct sandbox_serial_priv *priv = dev_get_priv(dev);

	if (!sandbox_serial_pending(dev, true))
		return -EAGAIN;	/* buffer empty */

	return membuff_getbyte(&priv->buf);
}

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
		.clock = SERIAL_DEFAULT_CLOCK,
	};

	if (!serial_info)
		return -EINVAL;

	*serial_info = info;

	return 0;
}

static const char * const ansi_colour[] = {
	"black", "red", "green", "yellow", "blue", "megenta", "cyan",
	"white",
};

static int sandbox_serial_of_to_plat(struct udevice *dev)
{
	struct sandbox_serial_plat *plat = dev_get_plat(dev);
	const char *colour;
	int i;

	if (CONFIG_IS_ENABLED(OF_PLATDATA))
		return 0;
	plat->colour = -1;
	colour = dev_read_string(dev, "sandbox,text-colour");
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
	.puts = sandbox_serial_puts,
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

U_BOOT_DRIVER(sandbox_serial) = {
	.name	= "sandbox_serial",
	.id	= UCLASS_SERIAL,
	.of_match = sandbox_serial_ids,
	.of_to_plat = sandbox_serial_of_to_plat,
	.plat_auto	= sizeof(struct sandbox_serial_plat),
	.priv_auto	= sizeof(struct sandbox_serial_priv),
	.probe = sandbox_serial_probe,
	.remove = sandbox_serial_remove,
	.ops	= &sandbox_serial_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

#if CONFIG_IS_ENABLED(OF_REAL) && !CONFIG_IS_ENABLED(OF_PLATDATA)
static const struct sandbox_serial_plat platdata_non_fdt = {
	.colour = -1,
};

U_BOOT_DRVINFO(serial_sandbox_non_fdt) = {
	.name = "sandbox_serial",
	.plat = &platdata_non_fdt,
};
#endif
