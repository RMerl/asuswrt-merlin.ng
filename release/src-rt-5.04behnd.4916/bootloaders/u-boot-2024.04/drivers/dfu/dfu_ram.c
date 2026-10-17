// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2013
 * Afzal Mohammed <afzal.mohd.ma@gmail.com>
 *
 * Reference: dfu_mmc.c
 * Copyright (C) 2012 Samsung Electronics
 * author: Lukasz Majewski <l.majewski@samsung.com>
 */

#include <common.h>
#include <malloc.h>
#include <mapmem.h>
#include <errno.h>
#include <dfu.h>
#include <linux/printk.h>

static int dfu_transfer_medium_ram(enum dfu_op op, struct dfu_entity *dfu,
				   u64 offset, void *buf, long *len)
{
	if (dfu->layout != DFU_RAM_ADDR) {
		pr_err("unsupported layout: %s\n", dfu_get_layout(dfu->layout));
		return  -EINVAL;
	}

	if (offset > dfu->data.ram.size) {
		pr_err("request exceeds allowed area\n");
		return -EINVAL;
	}

	if (op == DFU_OP_WRITE)
		memcpy(map_sysmem(dfu->data.ram.start + offset, 0), buf, *len);
	else
		memcpy(buf, map_sysmem(dfu->data.ram.start + offset, 0), *len);

	return 0;
}

static int dfu_write_medium_ram(struct dfu_entity *dfu, u64 offset,
				void *buf, long *len)
{
	return dfu_transfer_medium_ram(DFU_OP_WRITE, dfu, offset, buf, len);
}

int dfu_get_medium_size_ram(struct dfu_entity *dfu, u64 *size)
{
	*size = dfu->data.ram.size;

	return 0;
}

static int dfu_read_medium_ram(struct dfu_entity *dfu, u64 offset,
			       void *buf, long *len)
{
	return dfu_transfer_medium_ram(DFU_OP_READ, dfu, offset, buf, len);
}

int dfu_fill_entity_ram(struct dfu_entity *dfu, char *devstr, char **argv, int argc)
{
	char *s;

	if (argc != 3) {
		pr_err("Invalid number of arguments.\n");
		return -EINVAL;
	}

	dfu->dev_type = DFU_DEV_RAM;
	if (strcmp(argv[0], "ram")) {
		pr_err("unsupported device: %s\n", argv[0]);
		return -ENODEV;
	}

	dfu->layout = DFU_RAM_ADDR;
	dfu->data.ram.start = hextoul(argv[1], &s);
	if (*s)
		return -EINVAL;
	dfu->data.ram.size = hextoul(argv[2], &s);
	if (*s)
		return -EINVAL;

	dfu->write_medium = dfu_write_medium_ram;
	dfu->get_medium_size = dfu_get_medium_size_ram;
	dfu->read_medium = dfu_read_medium_ram;

	dfu->inited = 0;

	return 0;
}
