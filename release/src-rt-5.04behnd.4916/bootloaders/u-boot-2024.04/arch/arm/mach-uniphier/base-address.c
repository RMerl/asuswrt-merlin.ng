// SPDX-License-Identifier: GPL-2.0-only
//
// Copyright (C) 2019 Socionext Inc.
//   Author: Masahiro Yamada <yamada.masahiro@socionext.com>

#include <dm/of.h>
#include <fdt_support.h>
#include <linux/errno.h>
#include <linux/io.h>
#include <linux/libfdt.h>
#include <linux/sizes.h>
#include <asm/global_data.h>

#include "base-address.h"
#include "sc64-regs.h"
#include "sg-regs.h"

/*
 * Dummy initializers are needed to allocate these to .data section instead of
 * .bss section. The .bss section is unusable before relocation because the
 * .bss section and DT share the same address. Without the initializers,
 * DT would be broken.
 */
void __iomem *sc_base = (void *)0xdeadbeef;
void __iomem *sg_base = (void *)0xdeadbeef;

static u64 uniphier_base_address_get(const char *compat_tail)
{
	DECLARE_GLOBAL_DATA_PTR;
	const void *fdt = gd->fdt_blob;
	int offset, len, i;
	const char *str;

	for (offset = fdt_next_node(fdt, 0, NULL);
	     offset >= 0;
	     offset = fdt_next_node(fdt, offset, NULL)) {
		for (i = 0;
		     (str = fdt_stringlist_get(fdt, offset, "compatible", i, &len));
		     i++) {
			if (!memcmp(compat_tail,
				    str + len - strlen(compat_tail),
				    strlen(compat_tail)))
				return fdt_get_base_address(fdt, offset);
		}
	}

	return OF_BAD_ADDR;
}

int uniphier_base_address_init(void)
{
	u64 base;

	base = uniphier_base_address_get("-soc-glue");
	if (base == OF_BAD_ADDR)
		return -EINVAL;

	sg_base = ioremap(base, SZ_8K);

	base = uniphier_base_address_get("-sysctrl");
	if (base == OF_BAD_ADDR)
		return -EINVAL;

	sc_base = ioremap(base, SZ_64K);

	return 0;
}
