/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#include <config.h>
#include <linux/linkage.h>
#include <system-constants.h>

ENTRY(lowlevel_init)
	ldr	sp, = SYS_INIT_SP_ADDR
	b	uniphier_cache_disable
ENDPROC(lowlevel_init)
