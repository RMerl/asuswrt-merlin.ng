/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2009-2011 Freescale Semiconductor, Inc.
 */

#ifndef _FSL_PORTALS_H_
#define _FSL_PORTALS_H_

/* entries must be in order and contiguous */
enum fsl_dpaa_dev {
	FSL_HW_PORTAL_SEC,
#ifdef CONFIG_SYS_DPAA_FMAN
	FSL_HW_PORTAL_FMAN1,
#if (CONFIG_SYS_NUM_FMAN == 2)
	FSL_HW_PORTAL_FMAN2,
#endif
#endif
	FSL_HW_PORTAL_PME,
#ifdef CONFIG_SYS_FSL_RAID_ENGINE
	FSL_HW_PORTAL_RAID_ENGINE,
#endif
#ifdef CONFIG_SYS_DPAA_RMAN
	FSL_HW_PORTAL_RMAN,
#endif
#ifdef CONFIG_SYS_DPAA_DCE
	FSL_HW_PORTAL_DCE,
#endif

};

struct qportal_info {
	u16	dliodn;	/* DQRR LIODN */
	u16	fliodn;	/* frame data LIODN */
	u16	liodn_offset;
	u8	sdest;
};

#define SET_QP_INFO(dqrr, fdata, off, dest) \
	{ .dliodn = dqrr, .fliodn = fdata, .liodn_offset = off, .sdest = dest }

extern int get_dpaa_liodn(enum fsl_dpaa_dev dpaa_dev,
			  u32 *liodns, int liodn_offset);
extern struct qportal_info qp_info[];
extern void fdt_portal(void *blob, const char *compat, const char *container,
			u64 addr, u32 size);

#endif
