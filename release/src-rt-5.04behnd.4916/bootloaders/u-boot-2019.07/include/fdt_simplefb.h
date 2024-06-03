/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Simplefb device tree support
 *
 * (C) Copyright 2015
 * Stephen Warren <swarren@wwwdotorg.org>
 */

#ifndef _FDT_SIMPLEFB_H_
#define _FDT_SIMPLEFB_H_
int lcd_dt_simplefb_add_node(void *blob);
int lcd_dt_simplefb_enable_existing_node(void *blob);
#endif
