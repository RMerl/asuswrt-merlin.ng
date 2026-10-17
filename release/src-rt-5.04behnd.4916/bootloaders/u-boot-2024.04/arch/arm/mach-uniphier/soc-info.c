// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#include <linux/bitfield.h>
#include <linux/io.h>
#include <linux/types.h>

#include "sg-regs.h"
#include "soc-info.h"

unsigned int uniphier_get_soc_id(void)
{
	u32 rev = readl(sg_base + SG_REVISION);

	return FIELD_GET(SG_REVISION_TYPE_MASK, rev);
}

unsigned int uniphier_get_soc_model(void)
{
	u32 rev = readl(sg_base + SG_REVISION);

	return FIELD_GET(SG_REVISION_MODEL_MASK, rev);
}

unsigned int uniphier_get_soc_revision(void)
{
	u32 rev = readl(sg_base + SG_REVISION);

	return FIELD_GET(SG_REVISION_REV_MASK, rev);
}
