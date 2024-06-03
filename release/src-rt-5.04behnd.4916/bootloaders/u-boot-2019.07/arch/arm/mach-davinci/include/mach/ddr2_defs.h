/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2011
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 */
#ifndef _DV_DDR2_DEFS_H_
#define _DV_DDR2_DEFS_H_

/*
 * DDR2 Memory Ctrl Register structure
 * See sprueh7d.pdf for more details.
 */
struct dv_ddr2_regs_ctrl {
	unsigned char	rsvd0[4];	/* 0x00 */
	unsigned int	sdrstat;	/* 0x04 */
	unsigned int	sdbcr;		/* 0x08 */
	unsigned int	sdrcr;		/* 0x0C */
	unsigned int	sdtimr;		/* 0x10 */
	unsigned int	sdtimr2;	/* 0x14 */
	unsigned char	rsvd1[4];	/* 0x18 */
	unsigned int	sdbcr2;		/* 0x1C */
	unsigned int	pbbpr;		/* 0x20 */
	unsigned char	rsvd2[156];	/* 0x24 */
	unsigned int	irr;		/* 0xC0 */
	unsigned int	imr;		/* 0xC4 */
	unsigned int	imsr;		/* 0xC8 */
	unsigned int	imcr;		/* 0xCC */
	unsigned char	rsvd3[20];	/* 0xD0 */
	unsigned int	ddrphycr;	/* 0xE4 */
	unsigned int	ddrphycr2;	/* 0xE8 */
	unsigned char	rsvd4[4];	/* 0xEC */
};

#define DV_DDR_PHY_PWRDNEN		0x40
#define DV_DDR_PHY_EXT_STRBEN	0x80
#define DV_DDR_PHY_RD_LATENCY_SHIFT	0

#define DV_DDR_SDTMR1_RFC_SHIFT	25
#define DV_DDR_SDTMR1_RP_SHIFT	22
#define DV_DDR_SDTMR1_RCD_SHIFT	19
#define DV_DDR_SDTMR1_WR_SHIFT	16
#define DV_DDR_SDTMR1_RAS_SHIFT	11
#define DV_DDR_SDTMR1_RC_SHIFT	6
#define DV_DDR_SDTMR1_RRD_SHIFT	3
#define DV_DDR_SDTMR1_WTR_SHIFT	0

#define DV_DDR_SDTMR2_RASMAX_SHIFT	27
#define DV_DDR_SDTMR2_XP_SHIFT	25
#define DV_DDR_SDTMR2_ODT_SHIFT	23
#define DV_DDR_SDTMR2_XSNR_SHIFT	16
#define DV_DDR_SDTMR2_XSRD_SHIFT	8
#define DV_DDR_SDTMR2_RTP_SHIFT	5
#define DV_DDR_SDTMR2_CKE_SHIFT	0

#define DV_DDR_SDCR_DDR2TERM1_SHIFT	27
#define DV_DDR_SDCR_IBANK_POS_SHIFT	26
#define DV_DDR_SDCR_MSDRAMEN_SHIFT	25
#define DV_DDR_SDCR_DDRDRIVE1_SHIFT	24
#define DV_DDR_SDCR_BOOTUNLOCK_SHIFT	23
#define DV_DDR_SDCR_DDR_DDQS_SHIFT	22
#define DV_DDR_SDCR_DDR2EN_SHIFT	20
#define DV_DDR_SDCR_DDRDRIVE0_SHIFT	18
#define DV_DDR_SDCR_DDREN_SHIFT	17
#define DV_DDR_SDCR_SDRAMEN_SHIFT	16
#define DV_DDR_SDCR_TIMUNLOCK_SHIFT	15
#define DV_DDR_SDCR_BUS_WIDTH_SHIFT	14
#define DV_DDR_SDCR_CL_SHIFT		9
#define DV_DDR_SDCR_IBANK_SHIFT	4
#define DV_DDR_SDCR_PAGESIZE_SHIFT	0

#define DV_DDR_SDRCR_LPMODEN	(1 << 31)
#define DV_DDR_SDRCR_MCLKSTOPEN	(1 << 30)

#define DV_DDR_SRCR_LPMODEN_SHIFT	31
#define DV_DDR_SRCR_MCLKSTOPEN_SHIFT	30

#define DV_DDR_BOOTUNLOCK	(1 << DV_DDR_SDCR_BOOTUNLOCK_SHIFT)
#define DV_DDR_TIMUNLOCK	(1 << DV_DDR_SDCR_TIMUNLOCK_SHIFT)

#define dv_ddr2_regs_ctrl \
	((struct dv_ddr2_regs_ctrl *)DAVINCI_DDR_EMIF_CTRL_BASE)

#endif /* _DV_DDR2_DEFS_H_ */
