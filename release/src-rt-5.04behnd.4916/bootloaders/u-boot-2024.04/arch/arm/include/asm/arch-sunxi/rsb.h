/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2014 Hans de Goede <hdegoede@redhat.com>
 *
 * Based on allwinner u-boot sources rsb code which is:
 * (C) Copyright 2007-2013
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * lixiang <lixiang@allwinnertech.com>
 */

#ifndef __SUNXI_RSB_H
#define __SUNXI_RSB_H

#include <asm/io.h>

struct sunxi_rsb_reg {
	u32 ctrl;	/* 0x00 */
	u32 ccr;	/* 0x04 */
	u32 inte;	/* 0x08 */
	u32 stat;	/* 0x0c */
	u32 addr;	/* 0x10 */
	u8 res0[8];	/* 0x14 */
	u32 data;	/* 0x1c */
	u8 res1[4];	/* 0x20 */
	u32 lcr;	/* 0x24 */
	u32 dmcr;	/* 0x28 */
	u32 cmd;	/* 0x2c */
	u32 devaddr;	/* 0x30 */
};

#define RSB_CTRL_SOFT_RST		(1 << 0)
#define RSB_CTRL_START_TRANS		(1 << 7)

#define RSB_STAT_TOVER_INT		(1 << 0)
#define RSB_STAT_TERR_INT		(1 << 1)
#define RSB_STAT_LBSY_INT		(1 << 2)

#define RSB_DMCR_DEVICE_MODE_DATA	0x7c3e00
#define RSB_DMCR_DEVICE_MODE_START	(1 << 31)

#define RSB_CMD_BYTE_WRITE		0x4e
#define RSB_CMD_BYTE_READ		0x8b
#define RSB_CMD_SET_RTSADDR		0xe8

#define RSB_DEVADDR_RUNTIME_ADDR(x)	((x) << 16)
#define RSB_DEVADDR_DEVICE_ADDR(x)	((x) << 0)

int rsb_init(void);
int rsb_set_device_address(u16 device_addr, u16 runtime_addr);
int rsb_write(const u16 runtime_device_addr, const u8 reg_addr, u8 data);
int rsb_read(const u16 runtime_device_addr, const u8 reg_addr, u8 *data);

#endif
