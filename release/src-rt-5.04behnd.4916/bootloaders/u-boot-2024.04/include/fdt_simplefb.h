/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Simplefb device tree support
 *
 * (C) Copyright 2015
 * Stephen Warren <swarren@wwwdotorg.org>
 */

#ifndef _FDT_SIMPLEFB_H_
#define _FDT_SIMPLEFB_H_
int fdt_simplefb_add_node(void *blob);
int fdt_simplefb_enable_and_mem_rsv(void *blob);
#endif
