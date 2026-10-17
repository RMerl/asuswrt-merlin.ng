// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2019, STMicroelectronics - All Rights Reserved
 */
#include <common.h>
#include <dfu.h>
#include <errno.h>
#include <log.h>
#include <malloc.h>

int __weak dfu_write_medium_virt(struct dfu_entity *dfu, u64 offset,
				 void *buf, long *len)
{
	debug("%s: off=0x%llx, len=0x%x\n", __func__, offset, (u32)*len);

	return 0;
}

int __weak dfu_get_medium_size_virt(struct dfu_entity *dfu, u64 *size)
{
	*size = 0;

	return 0;
}

int __weak dfu_read_medium_virt(struct dfu_entity *dfu, u64 offset,
				void *buf, long *len)
{
	debug("%s: off=0x%llx, len=0x%x\n", __func__, offset, (u32)*len);
	*len = 0;

	return 0;
}

int dfu_fill_entity_virt(struct dfu_entity *dfu, char *devstr, char **argv, int argc)
{
	debug("%s: devstr = %s\n", __func__, devstr);

	if (argc != 0)
		return -EINVAL;

	dfu->dev_type = DFU_DEV_VIRT;
	dfu->layout = DFU_RAW_ADDR;
	dfu->data.virt.dev_num = dectoul(devstr, NULL);

	dfu->write_medium = dfu_write_medium_virt;
	dfu->get_medium_size = dfu_get_medium_size_virt;
	dfu->read_medium = dfu_read_medium_virt;

	dfu->inited = 0;

	return 0;
}
