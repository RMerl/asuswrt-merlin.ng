// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2012 Nobuhiro Iwamatsu <nobuhiro.iwamatsu.yj@renesas.com>
 * (C) Copyright 2012 Renesas Solutions Corp.
 */
#include <common.h>
#include <asm/io.h>

#ifdef CONFIG_ARCH_RMOBILE_BOARD_STRING
int checkboard(void)
{
	printf("Board: %s\n", CONFIG_ARCH_RMOBILE_BOARD_STRING);
	return 0;
}
#endif
