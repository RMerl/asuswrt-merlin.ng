/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2013 Broadcom Corporation.
 */

/* This API file is loosely based on u-boot/drivers/video/ipu.h and linux */

#ifndef __KONA_COMMON_CLK_H
#define __KONA_COMMON_CLK_H

#include <linux/types.h>

struct clk;

/* Only implement required functions for your specific architecture */
int clk_init(void);
struct clk *clk_get(const char *id);
int clk_enable(struct clk *clk);
void clk_disable(struct clk *clk);
unsigned long clk_get_rate(struct clk *clk);
long clk_round_rate(struct clk *clk, unsigned long rate);
int clk_set_rate(struct clk *clk, unsigned long rate);
int clk_set_parent(struct clk *clk, struct clk *parent);
struct clk *clk_get_parent(struct clk *clk);
int clk_sdio_enable(void *base, u32 rate, u32 *actual_ratep);
int clk_bsc_enable(void *base);
int clk_usb_otg_enable(void *base);

#endif
