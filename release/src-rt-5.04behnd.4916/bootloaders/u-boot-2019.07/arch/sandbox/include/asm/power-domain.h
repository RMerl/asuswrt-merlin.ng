/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2016, NVIDIA CORPORATION.
 */

#ifndef __SANDBOX_POWER_DOMAIN_H
#define __SANDBOX_POWER_DOMAIN_H

#include <common.h>

struct udevice;

int sandbox_power_domain_query(struct udevice *dev, unsigned long id);

int sandbox_power_domain_test_get(struct udevice *dev);
int sandbox_power_domain_test_on(struct udevice *dev);
int sandbox_power_domain_test_off(struct udevice *dev);
int sandbox_power_domain_test_free(struct udevice *dev);

#endif
