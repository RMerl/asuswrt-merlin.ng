/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2013-2019 Arcturus Networks, Inc.
 *           https://www.arcturusnetworks.com/products/ucp1020/
 *           by Oleksandr G Zhadan et al.
 */

#ifndef __UCP1020_H__
#define __UCP1020_H__

#define GPIO0		31
#define GPIO1		30
#define GPIO2		29
#define GPIO3		28
#define GPIO4		27
#define GPIO5		26
#define GPIO6		25
#define GPIO7		24
#define GPIO8		23
#define GPIO9		22
#define GPIO10		21
#define GPIO11		20
#define GPIO12		19
#define GPIO13		18
#define GPIO14		17
#define GPIO15		16
#define GPIO_MAX_NUM	16

#define GPIO_SDHC_CD	GPIO8
#define GPIO_SDHC_WP	GPIO9
#define GPIO_USB_PCTL0	GPIO10
#define GPIO_PCIE1_EN	GPIO11
#define GPIO_PCIE2_EN	GPIO10
#define GPIO_USB_PCTL1	GPIO11

#define GPIO_WD		GPIO15

#ifdef CONFIG_MMC
static char *defkargs = "root=/dev/mtdblock1 rootfstype=cramfs ro";
static char *mmckargs = "root=/dev/mmcblk0p1 rootwait rw";
#endif

int get_arc_info(void);

#endif
