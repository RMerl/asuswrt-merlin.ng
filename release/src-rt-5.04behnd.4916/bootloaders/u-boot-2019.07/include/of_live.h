/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2017 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 *
 * Support for a 'live' (as opposed to flat) device tree
 */

#ifndef _OF_LIVE_H
#define _OF_LIVE_H

struct device_node;

/**
 * of_live_build() - build a live (hierarchical) tree from a flat DT
 *
 * @fdt_blob: Input tree to convert
 * @rootp: Returns live tree that was created
 * @return 0 if OK, -ve on error
 */
int of_live_build(const void *fdt_blob, struct device_node **rootp);

#endif
