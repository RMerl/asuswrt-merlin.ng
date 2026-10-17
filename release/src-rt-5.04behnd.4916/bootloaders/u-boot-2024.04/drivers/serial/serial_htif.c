// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022 Ventana Micro Systems Inc.
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <fdtdec.h>
#include <log.h>
#include <watchdog.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <linux/compiler.h>
#include <serial.h>
#include <linux/err.h>

DECLARE_GLOBAL_DATA_PTR;

#define HTIF_DATA_BITS		48
#define HTIF_DATA_MASK		((1ULL << HTIF_DATA_BITS) - 1)
#define HTIF_DATA_SHIFT		0
#define HTIF_CMD_BITS		8
#define HTIF_CMD_MASK		((1ULL << HTIF_CMD_BITS) - 1)
#define HTIF_CMD_SHIFT		48
#define HTIF_DEV_BITS		8
#define HTIF_DEV_MASK		((1ULL << HTIF_DEV_BITS) - 1)
#define HTIF_DEV_SHIFT		56

#define HTIF_DEV_SYSTEM		0
#define HTIF_DEV_CONSOLE	1

#define HTIF_CONSOLE_CMD_GETC	0
#define HTIF_CONSOLE_CMD_PUTC	1

#if __riscv_xlen == 64
# define TOHOST_CMD(dev, cmd, payload) \
	(((u64)(dev) << HTIF_DEV_SHIFT) | \
	 ((u64)(cmd) << HTIF_CMD_SHIFT) | \
	 (u64)(payload))
#else
# define TOHOST_CMD(dev, cmd, payload) ({ \
	if ((dev) || (cmd)) \
		__builtin_trap(); \
	(payload); })
#endif
#define FROMHOST_DEV(fromhost_value) \
	((u64)((fromhost_value) >> HTIF_DEV_SHIFT) & HTIF_DEV_MASK)
#define FROMHOST_CMD(fromhost_value) \
	((u64)((fromhost_value) >> HTIF_CMD_SHIFT) & HTIF_CMD_MASK)
#define FROMHOST_DATA(fromhost_value) \
	((u64)((fromhost_value) >> HTIF_DATA_SHIFT) & HTIF_DATA_MASK)

struct htif_plat {
	void *fromhost;
	void *tohost;
	int console_char;
};

static void __check_fromhost(struct htif_plat *plat)
{
	u64 fh = readq(plat->fromhost);

	if (!fh)
		return;
	writeq(0, plat->fromhost);

	/* this should be from the console */
	if (FROMHOST_DEV(fh) != HTIF_DEV_CONSOLE)
		__builtin_trap();
	switch (FROMHOST_CMD(fh)) {
	case HTIF_CONSOLE_CMD_GETC:
		plat->console_char = 1 + (u8)FROMHOST_DATA(fh);
		break;
	case HTIF_CONSOLE_CMD_PUTC:
		break;
	default:
		__builtin_trap();
	}
}

static void __set_tohost(struct htif_plat *plat,
			 u64 dev, u64 cmd, u64 data)
{
	while (readq(plat->tohost))
		__check_fromhost(plat);
	writeq(TOHOST_CMD(dev, cmd, data), plat->tohost);
}

static int htif_serial_putc(struct udevice *dev, const char ch)
{
	struct htif_plat *plat = dev_get_plat(dev);

	__set_tohost(plat, HTIF_DEV_CONSOLE, HTIF_CONSOLE_CMD_PUTC, ch);
	return 0;
}

static int htif_serial_getc(struct udevice *dev)
{
	int ch;
	struct htif_plat *plat = dev_get_plat(dev);

	if (plat->console_char < 0)
		__check_fromhost(plat);

	if (plat->console_char >= 0) {
		ch = plat->console_char;
		plat->console_char = -1;
		__set_tohost(plat, HTIF_DEV_CONSOLE, HTIF_CONSOLE_CMD_GETC, 0);
		return (ch) ? ch - 1 : -EAGAIN;
	}

	return -EAGAIN;
}

static int htif_serial_pending(struct udevice *dev, bool input)
{
	struct htif_plat *plat = dev_get_plat(dev);

	if (!input)
		return 0;

	if (plat->console_char < 0)
		__check_fromhost(plat);

	return (plat->console_char >= 0) ? 1 : 0;
}

static int htif_serial_probe(struct udevice *dev)
{
	struct htif_plat *plat = dev_get_plat(dev);

	/* Queue first getc request */
	__set_tohost(plat, HTIF_DEV_CONSOLE, HTIF_CONSOLE_CMD_GETC, 0);

	return 0;
}

static int htif_serial_of_to_plat(struct udevice *dev)
{
	fdt_addr_t addr;
	struct htif_plat *plat = dev_get_plat(dev);

	addr = dev_read_addr_index(dev, 0);
	if (addr == FDT_ADDR_T_NONE)
		return -ENODEV;
	plat->fromhost = (void *)(uintptr_t)addr;
	plat->tohost = plat->fromhost + sizeof(u64);

	addr = dev_read_addr_index(dev, 1);
	if (addr != FDT_ADDR_T_NONE)
		plat->tohost = (void *)(uintptr_t)addr;

	plat->console_char = -1;

	return 0;
}

static const struct dm_serial_ops htif_serial_ops = {
	.putc = htif_serial_putc,
	.getc = htif_serial_getc,
	.pending = htif_serial_pending,
};

static const struct udevice_id htif_serial_ids[] = {
	{ .compatible = "ucb,htif0" },
	{ }
};

U_BOOT_DRIVER(serial_htif) = {
	.name		= "serial_htif",
	.id		= UCLASS_SERIAL,
	.of_match	= htif_serial_ids,
	.of_to_plat	= htif_serial_of_to_plat,
	.plat_auto	= sizeof(struct htif_plat),
	.probe		= htif_serial_probe,
	.ops		= &htif_serial_ops,
};
