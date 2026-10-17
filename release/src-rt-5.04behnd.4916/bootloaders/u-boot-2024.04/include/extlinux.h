/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2021 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef __extlinux_h
#define __extlinux_h

#define EXTLINUX_FNAME	"extlinux/extlinux.conf"

/**
 * struct extlinux_info - useful information for extlinux_getfile()
 *
 * @dev: bootmethod device being used to boot
 * @bflow: bootflow being booted
 */
struct extlinux_info {
	struct udevice *dev;
	struct bootflow *bflow;
	struct cmd_tbl *cmdtp;
};

#endif
