/*
 * Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 */

#include <common.h>
#include <cros_ec.h>
#include <dm.h>
#include <errno.h>
#include <log.h>

struct udevice *board_get_cros_ec_dev(void)
{
	struct udevice *dev;
	int ret;

	ret = uclass_get_device(UCLASS_CROS_EC, 0, &dev);
	if (ret) {
		debug("%s: Error %d\n", __func__, ret);
		return NULL;
	}
	return dev;
}
