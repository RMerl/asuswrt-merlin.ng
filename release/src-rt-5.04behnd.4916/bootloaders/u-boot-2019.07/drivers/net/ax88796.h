/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * AX88796L(NE2000) support
 *
 * (c) 2007 Nobuhiro Iwamatsu <iwamatsu@nigauri.org>
 */

#ifndef __DRIVERS_AX88796L_H__
#define __DRIVERS_AX88796L_H__

#define DP_DATA		(0x10 << 1)
#define START_PG	0x40	/* First page of TX buffer */
#define START_PG2	0x48
#define STOP_PG		0x80	/* Last page +1 of RX ring */
#define TX_PAGES	12
#define RX_START	(START_PG+TX_PAGES)
#define RX_END		STOP_PG

#define AX88796L_BASE_ADDRESS	CONFIG_DRIVER_NE2000_BASE
#define AX88796L_BYTE_ACCESS	0x00001000
#define AX88796L_OFFSET		0x00000400
#define AX88796L_ADDRESS_BYTE	AX88796L_BASE_ADDRESS + \
		AX88796L_BYTE_ACCESS + AX88796L_OFFSET
#define AX88796L_REG_MEMR	AX88796L_ADDRESS_BYTE + (0x14<<1)
#define AX88796L_REG_CR		AX88796L_ADDRESS_BYTE + (0x00<<1)

#define AX88796L_CR		(*(vu_short *)(AX88796L_REG_CR))
#define AX88796L_MEMR		(*(vu_short *)(AX88796L_REG_MEMR))

#define EECS_HIGH		(AX88796L_MEMR |= 0x10)
#define EECS_LOW		(AX88796L_MEMR &= 0xef)
#define EECLK_HIGH		(AX88796L_MEMR |= 0x80)
#define EECLK_LOW		(AX88796L_MEMR &= 0x7f)
#define EEDI_HIGH		(AX88796L_MEMR |= 0x20)
#define EEDI_LOW		(AX88796L_MEMR &= 0xdf)
#define EEDO			((AX88796L_MEMR & 0x40)>>6)

#define PAGE0_SET		(AX88796L_CR &= 0x3f)
#define PAGE1_SET		(AX88796L_CR = (AX88796L_CR & 0x3f) | 0x40)

#define BIT_DUMMY	0
#define MAC_EEP_READ	1
#define MAC_EEP_WRITE	2
#define MAC_EEP_ERACE	3
#define MAC_EEP_EWEN	4
#define MAC_EEP_EWDS	5

/* R7780MP Specific code */
#if defined(CONFIG_R7780MP)
#define ISA_OFFSET	0x1400
#define DP_IN(_b_, _o_, _d_)	(_d_) = \
	*( (vu_short *) ((_b_) + ((_o_) * 2) + ISA_OFFSET))
#define DP_OUT(_b_, _o_, _d_) \
	*((vu_short *)((_b_) + ((_o_) * 2) + ISA_OFFSET)) = (_d_)
#define DP_IN_DATA(_b_, _d_)	(_d_) = *( (vu_short *) ((_b_) + ISA_OFFSET))
#define DP_OUT_DATA(_b_, _d_)	*( (vu_short *) ((_b_)+ISA_OFFSET)) = (_d_)
#else
/* Please change for your target boards */
#define ISA_OFFSET	0x0000
#define DP_IN(_b_, _o_, _d_)	(_d_) = *( (vu_short *)((_b_)+(_o_ )+ISA_OFFSET))
#define DP_OUT(_b_, _o_, _d_)	*((vu_short *)((_b_)+(_o_)+ISA_OFFSET)) = (_d_)
#define DP_IN_DATA(_b_, _d_)	(_d_) = *( (vu_short *) ((_b_)+ISA_OFFSET))
#define DP_OUT_DATA(_b_, _d_)	*( (vu_short *) ((_b_)+ISA_OFFSET)) = (_d_)
#endif

#endif /* __DRIVERS_AX88796L_H__ */
