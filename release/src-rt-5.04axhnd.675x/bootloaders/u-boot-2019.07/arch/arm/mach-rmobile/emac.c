// SPDX-License-Identifier: GPL-2.0+
/*
 * RMOBILE EtherMAC initialization.
 *
 * Copyright (C) 2012  Renesas Solutions Corp.
 * Copyright (C) 2012  Nobuhiro Iwamatsu <nobuhiro.iwamatsu.yj@renesas.com>
 */

#include <common.h>
#include <linux/errno.h>
#include <netdev.h>

int cpu_eth_init(bd_t *bis)
{
	int ret = -ENODEV;
#ifdef CONFIG_SH_ETHER
	ret = sh_eth_initialize(bis);
#endif
	return ret;
}
