/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2017 Icenowy Zheng <icenowy@aosc.io>
 */

#ifndef __SIMPLEFB_COMMON_H
#define __SIMPLEFB_COMMON_H

/**
 * meson_simplefb_fdt_match() - match a meson simplefb node
 *
 * Match a meson simplefb device node with a specified pipeline, and
 * return its offset.
 *
 * @blob: device tree blob
 * @pipeline: display pipeline
 * Return: device node offset in blob, or negative values if failed
 */
int meson_simplefb_fdt_match(void *blob, const char *pipeline);

#endif
