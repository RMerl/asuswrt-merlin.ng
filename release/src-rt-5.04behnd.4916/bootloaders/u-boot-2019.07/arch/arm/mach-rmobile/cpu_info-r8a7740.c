// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2012 Nobuhiro Iwamatsu <nobuhiro.iwamatsu.yj@renesas.com>
 * (C) Copyright 2012 Renesas Solutions Corp.
 */
#include <common.h>
#include <asm/io.h>

u32 rmobile_get_cpu_type(void)
{
	u32 id;
	u32 type;
	struct r8a7740_hpb *hpb = (struct r8a7740_hpb *)HPB_BASE;

	id = readl(hpb->cccr);
	type = (id >> 8) & 0xFF;

	return type;
}

u32 rmobile_get_cpu_rev(void)
{
	u32 id;
	u32 rev;
	struct r8a7740_hpb *hpb = (struct r8a7740_hpb *)HPB_BASE;

	id = readl(hpb->cccr);
	rev = (id >> 4) & 0xF;

	return rev;
}
