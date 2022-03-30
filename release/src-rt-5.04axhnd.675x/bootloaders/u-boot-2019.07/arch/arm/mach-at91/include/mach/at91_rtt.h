/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2010
 * Reinhard Meyer, reinhard.meyer@emk-elektronik.de
 *
 * Real-time Timer
 * Based on AT91SAM9XE datasheet
 */

#ifndef AT91_RTT_H
#define AT91_RTT_H

#ifndef __ASSEMBLY__

typedef struct at91_rtt {
	u32	mr;	/* Mode Register   RW 0x00008000 */
	u32	ar;	/* Alarm Register  RW 0xFFFFFFFF */
	u32	vr;	/* Value Register  RO 0x00000000 */
	u32	sr;	/* Status Register RO 0x00000000 */
} at91_rtt_t;

#endif /* __ASSEMBLY__ */

#define AT91_RTT_MR_RTPRES	0x0000ffff
#define AT91_RTT_MR_ALMIEN	0x00010000
#define AT91_RTT_RTTINCIEN	0x00020000
#define AT91_RTT_RTTRST	0x00040000

#define AT91_RTT_SR_ALMS	0x00000001
#define AT91_RTT_SR_RTTINC	0x00000002

#endif
