// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 Rockchip Electronics Co., Ltd
 * Author: Andy Yan <andy.yan@rock-chips.com>
 */

#include <common.h>

#if !CONFIG_IS_ENABLED(SYS_DCACHE_OFF)
void enable_caches(void)
{
	/* Enable D-cache. I-cache is already enabled in start.S */
	dcache_enable();
}
#endif
