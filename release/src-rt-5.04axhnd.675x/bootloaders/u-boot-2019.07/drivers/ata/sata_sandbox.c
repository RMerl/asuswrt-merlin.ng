// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>

int init_sata(int dev)
{
	return 0;
}

int reset_sata(int dev)
{
	return 0;
}

int scan_sata(int dev)
{
	return 0;
}

ulong sata_read(int dev, ulong blknr, lbaint_t blkcnt, void *buffer)
{
	return 0;
}

ulong sata_write(int dev, ulong blknr, lbaint_t blkcnt, const void *buffer)
{
	return 0;
}
