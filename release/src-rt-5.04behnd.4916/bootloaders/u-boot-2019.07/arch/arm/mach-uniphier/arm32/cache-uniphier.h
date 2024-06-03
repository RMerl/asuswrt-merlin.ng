/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#ifndef __CACHE_UNIPHIER_H
#define __CACHE_UNIPHIER_H

#include <linux/types.h>

void uniphier_cache_prefetch_range(u32 start, u32 end, u32 ways);
void uniphier_cache_touch_range(u32 start, u32 end, u32 ways);
void uniphier_cache_touch_zero_range(u32 start, u32 end, u32 ways);
void uniphier_cache_inv_way(u32 ways);
void uniphier_cache_set_active_ways(int cpu, u32 active_ways);
void uniphier_cache_enable(void);
void uniphier_cache_disable(void);

#endif /* __CACHE_UNIPHIER_H */
