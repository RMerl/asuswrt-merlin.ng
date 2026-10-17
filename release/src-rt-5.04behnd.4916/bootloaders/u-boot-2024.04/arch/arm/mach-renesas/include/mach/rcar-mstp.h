/* SPDX-License-Identifier: GPL-2.0 */
/*
 * arch/arm/include/asm/arch-renesas/rcar-mstp.h
 *
 * Copyright (C) 2013, 2014 Nobuhiro Iwamatsu <nobuhiro.iwamatsu.yj@renesas.com>
 * Copyright (C) 2013, 2014 Renesas Electronics Corporation
 */

#ifndef __ASM_ARCH_RCAR_MSTP_H
#define __ASM_ARCH_RCAR_MSTP_H

#define mstp_setbits(type, addr, saddr, set) \
		out_##type((saddr), in_##type(addr) | (set))
#define mstp_clrbits(type, addr, saddr, clear) \
		out_##type((saddr), in_##type(addr) & ~(clear))
#define mstp_setclrbits(type, addr, set, clear) \
		out_##type((addr), (in_##type(addr) | (set)) & ~(clear))
#define mstp_setbits_le32(addr, saddr, set) \
		mstp_setbits(le32, addr, saddr, set)
#define mstp_clrbits_le32(addr, saddr, clear) \
		mstp_clrbits(le32, addr, saddr, clear)
#define mstp_setclrbits_le32(addr, set, clear) \
		mstp_setclrbits(le32, addr, set, clear)

#ifndef CFG_SMSTP0_ENA
#define CFG_SMSTP0_ENA	0x00
#endif
#ifndef CFG_SMSTP1_ENA
#define CFG_SMSTP1_ENA	0x00
#endif
#ifndef CFG_SMSTP2_ENA
#define CFG_SMSTP2_ENA	0x00
#endif
#ifndef CFG_SMSTP3_ENA
#define CFG_SMSTP3_ENA	0x00
#endif
#ifndef CFG_SMSTP4_ENA
#define CFG_SMSTP4_ENA	0x00
#endif
#ifndef CFG_SMSTP5_ENA
#define CFG_SMSTP5_ENA	0x00
#endif
#ifndef CFG_SMSTP6_ENA
#define CFG_SMSTP6_ENA	0x00
#endif
#ifndef CFG_SMSTP7_ENA
#define CFG_SMSTP7_ENA	0x00
#endif
#ifndef CFG_SMSTP8_ENA
#define CFG_SMSTP8_ENA	0x00
#endif
#ifndef CFG_SMSTP9_ENA
#define CFG_SMSTP9_ENA	0x00
#endif
#ifndef CFG_SMSTP10_ENA
#define CFG_SMSTP10_ENA	0x00
#endif
#ifndef CFG_SMSTP11_ENA
#define CFG_SMSTP11_ENA	0x00
#endif

#ifndef CFG_RMSTP0_ENA
#define CFG_RMSTP0_ENA	0x00
#endif
#ifndef CFG_RMSTP1_ENA
#define CFG_RMSTP1_ENA	0x00
#endif
#ifndef CFG_RMSTP2_ENA
#define CFG_RMSTP2_ENA	0x00
#endif
#ifndef CFG_RMSTP3_ENA
#define CFG_RMSTP3_ENA	0x00
#endif
#ifndef CFG_RMSTP4_ENA
#define CFG_RMSTP4_ENA	0x00
#endif
#ifndef CFG_RMSTP5_ENA
#define CFG_RMSTP5_ENA	0x00
#endif
#ifndef CFG_RMSTP6_ENA
#define CFG_RMSTP6_ENA	0x00
#endif
#ifndef CFG_RMSTP7_ENA
#define CFG_RMSTP7_ENA	0x00
#endif
#ifndef CFG_RMSTP8_ENA
#define CFG_RMSTP8_ENA	0x00
#endif
#ifndef CFG_RMSTP9_ENA
#define CFG_RMSTP9_ENA	0x00
#endif
#ifndef CFG_RMSTP10_ENA
#define CFG_RMSTP10_ENA	0x00
#endif
#ifndef CFG_RMSTP11_ENA
#define CFG_RMSTP11_ENA	0x00
#endif

struct mstp_ctl {
	u32 s_addr;
	u32 s_dis;
	u32 s_ena;
	u32 r_addr;
	u32 r_dis;
	u32 r_ena;
};

#endif /* __ASM_ARCH_RCAR_MSTP_H */
