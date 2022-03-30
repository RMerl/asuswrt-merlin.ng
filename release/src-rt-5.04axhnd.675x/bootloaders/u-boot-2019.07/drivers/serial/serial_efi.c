// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <debug_uart.h>
#include <dm.h>
#include <efi.h>
#include <efi_api.h>
#include <errno.h>
#include <fdtdec.h>
#include <linux/compiler.h>
#include <asm/io.h>
#include <serial.h>

/* Information about the efi console */
struct serial_efi_priv {
	struct efi_simple_text_input_protocol *con_in;
	struct efi_simple_text_output_protocol *con_out;
	struct efi_input_key key;
	bool have_key;
};

int serial_efi_setbrg(struct udevice *dev, int baudrate)
{
	return 0;
}

static int serial_efi_get_key(struct serial_efi_priv *priv)
{
	int ret;

	if (priv->have_key)
		return 0;
	ret = priv->con_in->read_key_stroke(priv->con_in, &priv->key);
	if (ret == EFI_NOT_READY)
		return -EAGAIN;
	else if (ret != EFI_SUCCESS)
		return -EIO;

	priv->have_key = true;

	return 0;
}

static int serial_efi_getc(struct udevice *dev)
{
	struct serial_efi_priv *priv = dev_get_priv(dev);
	int ret, ch;

	ret = serial_efi_get_key(priv);
	if (ret)
		return ret;

	priv->have_key = false;
	ch = priv->key.unicode_char;

	/*
	 * Unicode char 8 (for backspace) is never returned. Instead we get a
	 * key scan code of 8. Handle this so that backspace works correctly
	 * in the U-Boot command line.
	 */
	if (!ch && priv->key.scan_code == 8)
		ch = 8;
	debug(" [%x %x %x] ", ch, priv->key.unicode_char, priv->key.scan_code);

	return ch;
}

static int serial_efi_putc(struct udevice *dev, const char ch)
{
	struct serial_efi_priv *priv = dev_get_priv(dev);
	uint16_t ucode[2];
	int ret;

	ucode[0] = ch;
	ucode[1] = '\0';
	ret = priv->con_out->output_string(priv->con_out, ucode);
	if (ret)
		return -EIO;

	return 0;
}

static int serial_efi_pending(struct udevice *dev, bool input)
{
	struct serial_efi_priv *priv = dev_get_priv(dev);
	int ret;

	/* We assume that EFI will stall if its output buffer fills up */
	if (!input)
		return 0;

	ret = serial_efi_get_key(priv);
	if (ret == -EAGAIN)
		return 0;
	else if (ret)
		return ret;

	return 1;
}

/*
 * There is nothing to init here since the EFI console is already running by
 * the time we enter U-Boot.
 */
static inline void _debug_uart_init(void)
{
}

static inline void _debug_uart_putc(int ch)
{
	struct efi_system_table *sys_table = efi_get_sys_table();
	uint16_t ucode[2];

	ucode[0] = ch;
	ucode[1] = '\0';
	sys_table->con_out->output_string(sys_table->con_out, ucode);
}

DEBUG_UART_FUNCS

static int serial_efi_probe(struct udevice *dev)
{
	struct efi_system_table *table = efi_get_sys_table();
	struct serial_efi_priv *priv = dev_get_priv(dev);

	priv->con_in = table->con_in;
	priv->con_out = table->con_out;

	return 0;
}

static const struct dm_serial_ops serial_efi_ops = {
	.putc = serial_efi_putc,
	.getc = serial_efi_getc,
	.pending = serial_efi_pending,
	.setbrg = serial_efi_setbrg,
};

static const struct udevice_id serial_efi_ids[] = {
	{ .compatible = "efi,uart" },
	{ }
};

U_BOOT_DRIVER(serial_efi) = {
	.name	= "serial_efi",
	.id	= UCLASS_SERIAL,
	.of_match = serial_efi_ids,
	.priv_auto_alloc_size = sizeof(struct serial_efi_priv),
	.probe = serial_efi_probe,
	.ops	= &serial_efi_ops,
};
