// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2014 The Chromium OS Authors.
 */

#include <common.h>
#include <dm.h>
#include <environment.h>
#include <errno.h>
#include <os.h>
#include <serial.h>
#include <stdio_dev.h>
#include <watchdog.h>
#include <dm/lists.h>
#include <dm/device-internal.h>
#include <dm/of_access.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Table with supported baudrates (defined in config_xyz.h)
 */
static const unsigned long baudrate_table[] = CONFIG_SYS_BAUDRATE_TABLE;

#if !CONFIG_VAL(SYS_MALLOC_F_LEN)
#error "Serial is required before relocation - define CONFIG_$(SPL_)SYS_MALLOC_F_LEN to make this work"
#endif

#if CONFIG_IS_ENABLED(SERIAL_PRESENT)
static int serial_check_stdout(const void *blob, struct udevice **devp)
{
	int node;

	/* Check for a chosen console */
	node = fdtdec_get_chosen_node(blob, "stdout-path");
	if (node < 0) {
		const char *str, *p, *name;

		/*
		 * Deal with things like
		 *	stdout-path = "serial0:115200n8";
		 *
		 * We need to look up the alias and then follow it to the
		 * correct node.
		 */
		str = fdtdec_get_chosen_prop(blob, "stdout-path");
		if (str) {
			p = strchr(str, ':');
			name = fdt_get_alias_namelen(blob, str,
					p ? p - str : strlen(str));
			if (name)
				node = fdt_path_offset(blob, name);
		}
	}
	if (node < 0)
		node = fdt_path_offset(blob, "console");
	if (!uclass_get_device_by_of_offset(UCLASS_SERIAL, node, devp))
		return 0;

	/*
	 * If the console is not marked to be bound before relocation, bind it
	 * anyway.
	 */
	if (node > 0 && !lists_bind_fdt(gd->dm_root, offset_to_ofnode(node),
					devp, false)) {
		if (!device_probe(*devp))
			return 0;
	}

	return -ENODEV;
}

static void serial_find_console_or_panic(void)
{
	const void *blob = gd->fdt_blob;
	struct udevice *dev;
#ifdef CONFIG_SERIAL_SEARCH_ALL
	int ret;
#endif

	if (CONFIG_IS_ENABLED(OF_PLATDATA)) {
		uclass_first_device(UCLASS_SERIAL, &dev);
		if (dev) {
			gd->cur_serial_dev = dev;
			return;
		}
	} else if (CONFIG_IS_ENABLED(OF_CONTROL) && blob) {
		/* Live tree has support for stdout */
		if (of_live_active()) {
			struct device_node *np = of_get_stdout();

			if (np && !uclass_get_device_by_ofnode(UCLASS_SERIAL,
					np_to_ofnode(np), &dev)) {
				gd->cur_serial_dev = dev;
				return;
			}
		} else {
			if (!serial_check_stdout(blob, &dev)) {
				gd->cur_serial_dev = dev;
				return;
			}
		}
	}
	if (!SPL_BUILD || !CONFIG_IS_ENABLED(OF_CONTROL) || !blob) {
		/*
		 * Try to use CONFIG_CONS_INDEX if available (it is numbered
		 * from 1!).
		 *
		 * Failing that, get the device with sequence number 0, or in
		 * extremis just the first working serial device we can find.
		 * But we insist on having a console (even if it is silent).
		 */
#ifdef CONFIG_CONS_INDEX
#define INDEX (CONFIG_CONS_INDEX - 1)
#else
#define INDEX 0
#endif

#ifdef CONFIG_SERIAL_SEARCH_ALL
		if (!uclass_get_device_by_seq(UCLASS_SERIAL, INDEX, &dev) ||
		    !uclass_get_device(UCLASS_SERIAL, INDEX, &dev)) {
			if (dev->flags & DM_FLAG_ACTIVATED) {
				gd->cur_serial_dev = dev;
				return;
			}
		}

		/* Search for any working device */
		for (ret = uclass_first_device_check(UCLASS_SERIAL, &dev);
		     dev;
		     ret = uclass_next_device_check(&dev)) {
			if (!ret) {
				/* Device did succeed probing */
				gd->cur_serial_dev = dev;
				return;
			}
		}
#else
		if (!uclass_get_device_by_seq(UCLASS_SERIAL, INDEX, &dev) ||
		    !uclass_get_device(UCLASS_SERIAL, INDEX, &dev) ||
		    (!uclass_first_device(UCLASS_SERIAL, &dev) && dev)) {
			gd->cur_serial_dev = dev;
			return;
		}
#endif

#undef INDEX
	}

#ifdef CONFIG_REQUIRE_SERIAL_CONSOLE
	panic_str("No serial driver found");
#endif
}
#endif /* CONFIG_SERIAL_PRESENT */

/* Called prior to relocation */
int serial_init(void)
{
#if CONFIG_IS_ENABLED(SERIAL_PRESENT)
	serial_find_console_or_panic();
	gd->flags |= GD_FLG_SERIAL_READY;
#endif

	return 0;
}

/* Called after relocation */
void serial_initialize(void)
{
	serial_init();
}

static void _serial_putc(struct udevice *dev, char ch)
{
	struct dm_serial_ops *ops = serial_get_ops(dev);
	int err;

	if (ch == '\n')
		_serial_putc(dev, '\r');

	do {
		err = ops->putc(dev, ch);
	} while (err == -EAGAIN);
}

static void _serial_puts(struct udevice *dev, const char *str)
{
	while (*str)
		_serial_putc(dev, *str++);
}

static int __serial_getc(struct udevice *dev)
{
	struct dm_serial_ops *ops = serial_get_ops(dev);
	int err;

	do {
		err = ops->getc(dev);
		if (err == -EAGAIN)
			WATCHDOG_RESET();
	} while (err == -EAGAIN);

	return err >= 0 ? err : 0;
}

static int __serial_tstc(struct udevice *dev)
{
	struct dm_serial_ops *ops = serial_get_ops(dev);

	if (ops->pending)
		return ops->pending(dev, true);

	return 1;
}

#if CONFIG_IS_ENABLED(SERIAL_RX_BUFFER)
static int _serial_tstc(struct udevice *dev)
{
	struct serial_dev_priv *upriv = dev_get_uclass_priv(dev);

	/* Read all available chars into the RX buffer */
	while (__serial_tstc(dev)) {
		upriv->buf[upriv->wr_ptr++] = __serial_getc(dev);
		upriv->wr_ptr %= CONFIG_SERIAL_RX_BUFFER_SIZE;
	}

	return upriv->rd_ptr != upriv->wr_ptr ? 1 : 0;
}

static int _serial_getc(struct udevice *dev)
{
	struct serial_dev_priv *upriv = dev_get_uclass_priv(dev);
	char val;

	if (upriv->rd_ptr == upriv->wr_ptr)
		return __serial_getc(dev);

	val = upriv->buf[upriv->rd_ptr++];
	upriv->rd_ptr %= CONFIG_SERIAL_RX_BUFFER_SIZE;

	return val;
}

#else /* CONFIG_IS_ENABLED(SERIAL_RX_BUFFER) */

static int _serial_getc(struct udevice *dev)
{
	return __serial_getc(dev);
}

static int _serial_tstc(struct udevice *dev)
{
	return __serial_tstc(dev);
}
#endif /* CONFIG_IS_ENABLED(SERIAL_RX_BUFFER) */

void serial_putc(char ch)
{
	if (gd->cur_serial_dev)
		_serial_putc(gd->cur_serial_dev, ch);
}

void serial_puts(const char *str)
{
	if (gd->cur_serial_dev)
		_serial_puts(gd->cur_serial_dev, str);
}

int serial_getc(void)
{
	if (!gd->cur_serial_dev)
		return 0;

	return _serial_getc(gd->cur_serial_dev);
}

int serial_tstc(void)
{
	if (!gd->cur_serial_dev)
		return 0;

	return _serial_tstc(gd->cur_serial_dev);
}

void serial_setbrg(void)
{
	struct dm_serial_ops *ops;

	if (!gd->cur_serial_dev)
		return;

	ops = serial_get_ops(gd->cur_serial_dev);
	if (ops->setbrg)
		ops->setbrg(gd->cur_serial_dev, gd->baudrate);
}

int serial_getconfig(struct udevice *dev, uint *config)
{
	struct dm_serial_ops *ops;

	ops = serial_get_ops(dev);
	if (ops->getconfig)
		return ops->getconfig(dev, config);

	return 0;
}

int serial_setconfig(struct udevice *dev, uint config)
{
	struct dm_serial_ops *ops;

	ops = serial_get_ops(dev);
	if (ops->setconfig)
		return ops->setconfig(dev, config);

	return 0;
}

int serial_getinfo(struct udevice *dev, struct serial_device_info *info)
{
	struct dm_serial_ops *ops;

	if (!info)
		return -EINVAL;

	info->baudrate = gd->baudrate;

	ops = serial_get_ops(dev);
	if (ops->getinfo)
		return ops->getinfo(dev, info);

	return -EINVAL;
}

void serial_stdio_init(void)
{
}

#if defined(CONFIG_DM_STDIO)

#if CONFIG_IS_ENABLED(SERIAL_PRESENT)
static void serial_stub_putc(struct stdio_dev *sdev, const char ch)
{
	_serial_putc(sdev->priv, ch);
}
#endif

static void serial_stub_puts(struct stdio_dev *sdev, const char *str)
{
	_serial_puts(sdev->priv, str);
}

static int serial_stub_getc(struct stdio_dev *sdev)
{
	return _serial_getc(sdev->priv);
}

static int serial_stub_tstc(struct stdio_dev *sdev)
{
	return _serial_tstc(sdev->priv);
}
#endif

/**
 * on_baudrate() - Update the actual baudrate when the env var changes
 *
 * This will check for a valid baudrate and only apply it if valid.
 */
static int on_baudrate(const char *name, const char *value, enum env_op op,
	int flags)
{
	int i;
	int baudrate;

	switch (op) {
	case env_op_create:
	case env_op_overwrite:
		/*
		 * Switch to new baudrate if new baudrate is supported
		 */
		baudrate = simple_strtoul(value, NULL, 10);

		/* Not actually changing */
		if (gd->baudrate == baudrate)
			return 0;

		for (i = 0; i < ARRAY_SIZE(baudrate_table); ++i) {
			if (baudrate == baudrate_table[i])
				break;
		}
		if (i == ARRAY_SIZE(baudrate_table)) {
			if ((flags & H_FORCE) == 0)
				printf("## Baudrate %d bps not supported\n",
				       baudrate);
			return 1;
		}
		if ((flags & H_INTERACTIVE) != 0) {
			printf("## Switch baudrate to %d bps and press ENTER ...\n",
			       baudrate);
			udelay(50000);
		}

		gd->baudrate = baudrate;

		serial_setbrg();

		udelay(50000);

		if ((flags & H_INTERACTIVE) != 0)
			while (1) {
				if (getc() == '\r')
					break;
			}

		return 0;
	case env_op_delete:
		printf("## Baudrate may not be deleted\n");
		return 1;
	default:
		return 0;
	}
}
U_BOOT_ENV_CALLBACK(baudrate, on_baudrate);

#if CONFIG_IS_ENABLED(SERIAL_PRESENT)
static int serial_post_probe(struct udevice *dev)
{
	struct dm_serial_ops *ops = serial_get_ops(dev);
#ifdef CONFIG_DM_STDIO
	struct serial_dev_priv *upriv = dev_get_uclass_priv(dev);
	struct stdio_dev sdev;
#endif
	int ret;

#if defined(CONFIG_NEEDS_MANUAL_RELOC)
	if (ops->setbrg)
		ops->setbrg += gd->reloc_off;
	if (ops->getc)
		ops->getc += gd->reloc_off;
	if (ops->putc)
		ops->putc += gd->reloc_off;
	if (ops->pending)
		ops->pending += gd->reloc_off;
	if (ops->clear)
		ops->clear += gd->reloc_off;
	if (ops->getconfig)
		ops->getconfig += gd->reloc_off;
	if (ops->setconfig)
		ops->setconfig += gd->reloc_off;
#if CONFIG_POST & CONFIG_SYS_POST_UART
	if (ops->loop)
		ops->loop += gd->reloc_off;
#endif
	if (ops->getinfo)
		ops->getinfo += gd->reloc_off;
#endif
	/* Set the baud rate */
	if (ops->setbrg) {
		ret = ops->setbrg(dev, gd->baudrate);
		if (ret)
			return ret;
	}

#ifdef CONFIG_DM_STDIO
	if (!(gd->flags & GD_FLG_RELOC))
		return 0;
	memset(&sdev, '\0', sizeof(sdev));

	strncpy(sdev.name, dev->name, sizeof(sdev.name));
	sdev.flags = DEV_FLAGS_OUTPUT | DEV_FLAGS_INPUT | DEV_FLAGS_DM;
	sdev.priv = dev;
	sdev.putc = serial_stub_putc;
	sdev.puts = serial_stub_puts;
	sdev.getc = serial_stub_getc;
	sdev.tstc = serial_stub_tstc;

#if CONFIG_IS_ENABLED(SERIAL_RX_BUFFER)
	/* Allocate the RX buffer */
	upriv->buf = malloc(CONFIG_SERIAL_RX_BUFFER_SIZE);
#endif

	stdio_register_dev(&sdev, &upriv->sdev);
#endif
	return 0;
}

static int serial_pre_remove(struct udevice *dev)
{
#if CONFIG_IS_ENABLED(SYS_STDIO_DEREGISTER)
	struct serial_dev_priv *upriv = dev_get_uclass_priv(dev);

	if (stdio_deregister_dev(upriv->sdev, true))
		return -EPERM;
#endif

	return 0;
}

UCLASS_DRIVER(serial) = {
	.id		= UCLASS_SERIAL,
	.name		= "serial",
	.flags		= DM_UC_FLAG_SEQ_ALIAS,
	.post_probe	= serial_post_probe,
	.pre_remove	= serial_pre_remove,
	.per_device_auto_alloc_size = sizeof(struct serial_dev_priv),
};
#endif
