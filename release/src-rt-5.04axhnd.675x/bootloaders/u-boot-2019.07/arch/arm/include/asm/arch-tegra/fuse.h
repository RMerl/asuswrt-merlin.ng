/* SPDX-License-Identifier: GPL-2.0+ */
/*
 *  (C) Copyright 2010,2011
 *  NVIDIA Corporation <www.nvidia.com>
 */

#ifndef _FUSE_H_
#define _FUSE_H_

/* FUSE registers */
struct fuse_regs {
	u32 reserved0[64];		/* 0x00 - 0xFC: */
	u32 production_mode;		/* 0x100: FUSE_PRODUCTION_MODE */
	u32 reserved1[3];		/* 0x104 - 0x10c: */
	u32 sku_info;			/* 0x110 */
	u32 reserved2[13];		/* 0x114 - 0x144: */
	u32 fa;				/* 0x148: FUSE_FA */
	u32 reserved3[21];		/* 0x14C - 0x19C: */
	u32 security_mode;		/* 0x1A0: FUSE_SECURITY_MODE */
};

#endif	/* ifndef _FUSE_H_ */
