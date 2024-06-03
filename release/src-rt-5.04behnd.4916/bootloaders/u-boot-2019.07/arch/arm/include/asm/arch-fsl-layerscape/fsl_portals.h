/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2018 NXP
 */

#ifndef _FSL_PORTALS_H_
#define _FSL_PORTALS_H_

struct qportal_info {
	u16	dicid;	/* DQRR ICID */
	u16	ficid;	/* frame data ICID */
	u16	icid;
	u8	sdest;
};

#define SET_QP_INFO(streamid, dest) \
	{ .dicid = (streamid), .ficid = (streamid), .icid = (streamid), \
	.sdest = (dest) }

extern struct qportal_info qp_info[];
void fdt_portal(void *blob, const char *compat, const char *container,
		u64 addr, u32 size);

#endif
