/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 */

#ifndef __VID_H_
#define __VID_H_

#define IR36021_LOOP1_MANUAL_ID_OFFSET	0x6A
#define IR36021_LOOP1_VOUT_OFFSET	0x9A
#define IR36021_MFR_ID_OFFSET		0x92
#define IR36021_MFR_ID			0x43
#define IR36021_INTEL_MODE_OOFSET	0x14
#define IR36021_MODE_MASK		0x20
#define IR36021_INTEL_MODE		0x00
#define IR36021_AMD_MODE		0x20

/* step the IR regulator in 5mV increments */
#define IR_VDD_STEP_DOWN		5
#define IR_VDD_STEP_UP			5
int adjust_vdd(ulong vdd_override);

#endif  /* __VID_H_ */
