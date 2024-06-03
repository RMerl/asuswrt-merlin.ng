// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#include <linux/io.h>
#include <linux/types.h>

#include "sg-regs.h"
#include "soc-info.h"

static unsigned int __uniphier_get_revision_field(unsigned int mask,
						  unsigned int shift)
{
	u32 revision = readl(SG_REVISION);

	return (revision >> shift) & mask;
}

unsigned int uniphier_get_soc_id(void)
{
	return __uniphier_get_revision_field(0xff, 16);
}

unsigned int uniphier_get_soc_model(void)
{
	return __uniphier_get_revision_field(0x7, 8);
}

unsigned int uniphier_get_soc_revision(void)
{
	return __uniphier_get_revision_field(0x1f, 0);
}
