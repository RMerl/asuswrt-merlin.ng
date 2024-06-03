/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2016, NVIDIA CORPORATION.
 */

#ifndef __SANDBOX_MBOX_H
#define __SANDBOX_MBOX_H

#include <common.h>

#define SANDBOX_MBOX_PING_XOR 0x12345678

struct udevice;

int sandbox_mbox_test_get(struct udevice *dev);
int sandbox_mbox_test_send(struct udevice *dev, uint32_t msg);
int sandbox_mbox_test_recv(struct udevice *dev, uint32_t *msg);
int sandbox_mbox_test_free(struct udevice *dev);

#endif
