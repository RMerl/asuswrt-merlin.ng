/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2016, NVIDIA CORPORATION.
 */

#ifndef __SANDBOX_RESET_H
#define __SANDBOX_RESET_H

#include <common.h>

struct udevice;

int sandbox_reset_query(struct udevice *dev, unsigned long id);

int sandbox_reset_test_get(struct udevice *dev);
int sandbox_reset_test_get_bulk(struct udevice *dev);
int sandbox_reset_test_assert(struct udevice *dev);
int sandbox_reset_test_assert_bulk(struct udevice *dev);
int sandbox_reset_test_deassert(struct udevice *dev);
int sandbox_reset_test_deassert_bulk(struct udevice *dev);
int sandbox_reset_test_free(struct udevice *dev);
int sandbox_reset_test_release_bulk(struct udevice *dev);

#endif
