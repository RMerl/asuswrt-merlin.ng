/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * sdp.h - Serial Download Protocol
 *
 * Copyright (C) 2017 Toradex
 * Author: Stefan Agner <stefan.agner@toradex.com>
 */

#ifndef __SDP_H_
#define __SDP_H_

int sdp_init(struct udevice *udc);

#ifdef CONFIG_SPL_BUILD
#include <spl.h>

int spl_sdp_handle(struct udevice *udc, struct spl_image_info *spl_image,
		   struct spl_boot_device *bootdev);
#else
int sdp_handle(struct udevice *udc);
#endif

#endif /* __SDP_H_ */
