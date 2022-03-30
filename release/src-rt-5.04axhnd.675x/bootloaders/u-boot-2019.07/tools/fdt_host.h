/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2008 Semihalf
 */

#ifndef __FDT_HOST_H__
#define __FDT_HOST_H__

/* Make sure to include u-boot version of libfdt include files */
#include "../include/linux/libfdt.h"
#include "../include/fdt_support.h"

/**
 * fdt_remove_unused_strings() - Remove any unused strings from an FDT
 *
 * This creates a new device tree in @new with unused strings removed. The
 * called can then use fdt_pack() to minimise the space consumed.
 *
 * @old:	Old device tree blog
 * @new:	Place to put new device tree blob, which must be as large as
 *		@old
 * @return
 *	0, on success
 *	-FDT_ERR_BADOFFSET, corrupt device tree
 *	-FDT_ERR_NOSPACE, out of space, which should not happen unless there
 *		is something very wrong with the device tree input
 */
int fdt_remove_unused_strings(const void *old, void *new);

int fit_check_sign(const void *working_fdt, const void *key);

#endif /* __FDT_HOST_H__ */
