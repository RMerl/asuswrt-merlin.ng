/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2010
 * Reinhard Meyer, reinhard.meyer@emk-elektronik.de
 *
 * Enhanced Embedded Flash Controller
 * Based on AT91SAM9XE datasheet
 */

#ifndef AT91_EEFC_H
#define AT91_EEFC_H

#ifndef __ASSEMBLY__

typedef struct at91_eefc {
	u32	fmr;	/* Flash Mode Register RW */
	u32	fcr;	/* Flash Command Register WO */
	u32	fsr;	/* Flash Status Register RO */
	u32	frr;	/* Flash Result Register RO */
} at91_eefc_t;

#endif /* __ASSEMBLY__ */

#define AT91_EEFC_FMR_FWS_MASK	0x00000f00
#define AT91_EEFC_FMR_FRDY_BIT	0x00000001

#define AT91_EEFC_FCR_KEY		0x5a000000
#define AT91_EEFC_FCR_FARG_MASK	0x00ffff00
#define AT91_EEFC_FCR_FARG_SHIFT	8
#define AT91_EEFC_FCR_FCMD_GETD	0x0
#define AT91_EEFC_FCR_FCMD_WP		0x1
#define AT91_EEFC_FCR_FCMD_WPL		0x2
#define AT91_EEFC_FCR_FCMD_EWP		0x3
#define AT91_EEFC_FCR_FCMD_EWPL	0x4
#define AT91_EEFC_FCR_FCMD_EA		0x5
#define AT91_EEFC_FCR_FCMD_SLB		0x8
#define AT91_EEFC_FCR_FCMD_CLB		0x9
#define AT91_EEFC_FCR_FCMD_GLB		0xA
#define AT91_EEFC_FCR_FCMD_SGPB	0xB
#define AT91_EEFC_FCR_FCMD_CGPB	0xC
#define AT91_EEFC_FCR_FCMD_GGPB	0xD

#define AT91_EEFC_FSR_FRDY	1
#define AT91_EEFC_FSR_FCMDE	2
#define AT91_EEFC_FSR_FLOCKE	4

#endif
