/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2020-2021 SiFive, Inc.
 *
 * Author: Sagar Kadam <sagar.kadam@sifive.com>
 */

#ifndef __RESET_SIFIVE_H
#define __RESET_SIFIVE_H

int sifive_reset_bind(struct udevice *dev, ulong count);

#endif
