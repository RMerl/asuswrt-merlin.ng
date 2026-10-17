// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2021
 * Walter Schweizer <swwa@users.sourceforge.net>
 * Phil Sutter <phil@nwl.cc>
 */

#include <config.h>
#include <vsprintf.h>
#include <env.h>
#include <net.h>
#include <asm/setup.h>

#include "legacy.h"

static unsigned int syno_board_id(void)
{
#ifdef CONFIG_MACH_TYPE
	switch (CONFIG_MACH_TYPE) {
	case 527:
		return SYNO_DS109_ID;
	case 3036:
		return SYNO_AXP_4BAY_2BAY;
	}
#endif
	return 0;
}

static unsigned int usb_port_modes(void)
{
	unsigned int i, ret = 0;
	char var[32], *val;

	for (i = 0; i < USBPORT_MAX; i++) {
		snprintf(var, 32, "usb%dMode", i);
		val = env_get(var);

		if (!val || strcasecmp(val, "host"))
			continue;

		ret |= 1 << i;
	}
	return ret;
}

/* Support old kernels */
void setup_board_tags(struct tag **in_params)
{
	struct tag_mv_uboot *t;
	struct tag *params;
	int i;

	debug("Synology board tags...\n");

	params = *in_params;
	t = (struct tag_mv_uboot *)&params->u;

	t->uboot_version = VER_NUM | syno_board_id();
	t->tclk = CFG_SYS_TCLK;
	t->sysclk = CFG_SYS_TCLK * 2;
	t->isusbhost = usb_port_modes();

	for (i = 0; i < ETHADDR_MAX; i++) {
		char addrvar[16], mtuvar[16];

		sprintf(addrvar, i ? "eth%daddr" : "ethaddr", i);
		sprintf(mtuvar, i ? "eth%dmtu" : "ethmtu", i);

		eth_env_get_enetaddr(addrvar, t->macaddr[i]);
		t->mtu[i] = env_get_ulong(mtuvar, 10, 0);
	}

	params->hdr.tag = ATAG_MV_UBOOT;
	params->hdr.size = tag_size(tag_mv_uboot);
	params = tag_next(params);
	*in_params = params;
}
