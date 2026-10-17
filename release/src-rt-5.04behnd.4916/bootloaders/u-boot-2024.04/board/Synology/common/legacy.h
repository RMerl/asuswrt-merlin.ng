/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2021
 * Walter Schweizer <swwa@users.sourceforge.net>
 * Phil Sutter <phil@nwl.cc>
 */

#ifndef __SYNO_LEGACY_H
#define __SYNO_LEGACY_H

/* Marvell uboot parameters */
#define ATAG_MV_UBOOT 0x41000403
#define VER_NUM       0x03040400 /* 3.4.4 */

#define BOARD_ID_BASE 0x0
#define SYNO_DS109_ID (BOARD_ID_BASE + 0x15)
#define SYNO_AXP_4BAY_2BAY (0xf + 1)

#define ETHADDR_MAX	4
#define USBPORT_MAX	3

struct tag_mv_uboot {
	u32 uboot_version;
	u32 tclk;
	u32 sysclk;
	u32 isusbhost;
	u8 macaddr[ETHADDR_MAX][ETH_ALEN];
	u16 mtu[ETHADDR_MAX];
	u32 fw_image_base;
	u32 fw_image_size;
};

#endif /* __SYNO_LEGACY_H */
