/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * FlexBus Internal Memory Map
 *
 * Copyright (C) 2004-2007 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
 */

#ifndef __FLEXBUS_H
#define __FLEXBUS_H

/*********************************************************************
* FlexBus Chip Selects (FBCS)
*********************************************************************/
#ifdef CONFIG_M5235
typedef struct fbcs {
    u16 csar0;      /* Chip-select Address */
    u16 res1;
    u32 csmr0;      /* Chip-select Mask */
    u16 res2;
    u16 cscr0;      /* Chip-select Control */

    u16 csar1;
    u16 res3;
    u32 csmr1;
    u16 res4;
    u16 cscr1;

    u16 csar2;
    u16 res5;
    u32 csmr2;
    u16 res6;
    u16 cscr2;

    u16 csar3;
    u16 res7;
    u32 csmr3;
    u16 res8;
    u16 cscr3;

    u16 csar4;
    u16 res9;
    u32 csmr4;
    u16 res10;
    u16 cscr4;

    u16 csar5;
    u16 res11;
    u32 csmr5;
    u16 res12;
    u16 cscr5;

    u16 csar6;
    u16 res13;
    u32 csmr6;
    u16 res14;
    u16 cscr6;

    u16 csar7;
    u16 res15;
    u32 csmr7;
    u16 res16;
    u16 cscr7;
} fbcs_t;
#else
typedef struct fbcs {
	u32 csar0;		/* Chip-select Address */
	u32 csmr0;		/* Chip-select Mask */
	u32 cscr0;		/* Chip-select Control */
	u32 csar1;
	u32 csmr1;
	u32 cscr1;
	u32 csar2;
	u32 csmr2;
	u32 cscr2;
	u32 csar3;
	u32 csmr3;
	u32 cscr3;
	u32 csar4;
	u32 csmr4;
	u32 cscr4;
	u32 csar5;
	u32 csmr5;
	u32 cscr5;
	u32 csar6;
	u32 csmr6;
	u32 cscr6;
	u32 csar7;
	u32 csmr7;
	u32 cscr7;
} fbcs_t;
#endif

#define FBCS_CSAR_BA(x)			((x) & 0xFFFF0000)

#define FBCS_CSMR_BAM(x)		(((x) & 0xFFFF) << 16)
#define FBCS_CSMR_BAM_MASK		(0x0000FFFF)
#define FBCS_CSMR_BAM_4G		(0xFFFF0000)
#define FBCS_CSMR_BAM_2G		(0x7FFF0000)
#define FBCS_CSMR_BAM_1G		(0x3FFF0000)
#define FBCS_CSMR_BAM_1024M		(0x3FFF0000)
#define FBCS_CSMR_BAM_512M		(0x1FFF0000)
#define FBCS_CSMR_BAM_256M		(0x0FFF0000)
#define FBCS_CSMR_BAM_128M		(0x07FF0000)
#define FBCS_CSMR_BAM_64M		(0x03FF0000)
#define FBCS_CSMR_BAM_32M		(0x01FF0000)
#define FBCS_CSMR_BAM_16M		(0x00FF0000)
#define FBCS_CSMR_BAM_8M		(0x007F0000)
#define FBCS_CSMR_BAM_4M		(0x003F0000)
#define FBCS_CSMR_BAM_2M		(0x001F0000)
#define FBCS_CSMR_BAM_1M		(0x000F0000)
#define FBCS_CSMR_BAM_1024K		(0x000F0000)
#define FBCS_CSMR_BAM_512K		(0x00070000)
#define FBCS_CSMR_BAM_256K		(0x00030000)
#define FBCS_CSMR_BAM_128K		(0x00010000)
#define FBCS_CSMR_BAM_64K		(0x00000000)

#ifdef CONFIG_M5249
#define FBCS_CSMR_WP			(0x00000080)
#define FBCS_CSMR_AM			(0x00000040)
#define FBCS_CSMR_CI			(0x00000020)
#define FBCS_CSMR_SC			(0x00000010)
#define FBCS_CSMR_SD			(0x00000008)
#define FBCS_CSMR_UC			(0x00000004)
#define FBCS_CSMR_UD			(0x00000002)
#else
#define FBCS_CSMR_WP			(0x00000100)
#endif
#define FBCS_CSMR_V			(0x00000001)	/* Valid bit */

#ifdef CONFIG_M5235
#define FBCS_CSCR_SRWS(x)       (((x) & 0x3) << 14)
#define FBCS_CSCR_IWS(x)        (((x) & 0xF) << 10)
#define FBCS_CSCR_AA_ON         (1 << 8)
#define FBCS_CSCR_AA_OFF        (0 << 8)
#define FBCS_CSCR_PS_32         (0 << 6)
#define FBCS_CSCR_PS_16         (2 << 6)
#define FBCS_CSCR_PS_8          (1 << 6)
#define FBCS_CSCR_BEM_ON        (1 << 5)
#define FBCS_CSCR_BEM_OFF       (0 << 5)
#define FBCS_CSCR_BSTR_ON       (1 << 4)
#define FBCS_CSCR_BSTR_OFF      (0 << 4)
#define FBCS_CSCR_BSTW_ON       (1 << 3)
#define FBCS_CSCR_BSTW_OFF      (0 << 3)
#define FBCS_CSCR_SWWS(x)       (((x) & 0x7) << 0)
#else
#define FBCS_CSCR_SWS(x)		(((x) & 0x3F) << 26)
#define FBCS_CSCR_SWS_MASK		(0x03FFFFFF)
#define FBCS_CSCR_SWSEN			(0x00800000)
#define FBCS_CSCR_ASET(x)		(((x) & 0x03) << 20)
#define FBCS_CSCR_ASET_MASK		(0xFFCFFFFF)
#define FBCS_CSCR_RDAH(x)		(((x) & 0x03) << 18)
#define FBCS_CSCR_RDAH_MASK		(0xFFF3FFFF)
#define FBCS_CSCR_WRAH(x)		(((x) & 0x03) << 16)
#define FBCS_CSCR_WRAH_MASK		(0xFFFCFFFF)
#define FBCS_CSCR_WS(x)			(((x) & 0x3F) << 10)
#define FBCS_CSCR_WS_MASK		(0xFFFF03FF)
#define FBCS_CSCR_SBM			(0x00000200)
#define FBCS_CSCR_AA			(0x00000100)
#define FBCS_CSCR_PS(x)			(((x) & 0x03) << 6)
#define FBCS_CSCR_PS_MASK		(0xFFFFFF3F)
#define FBCS_CSCR_BEM			(0x00000020)
#define FBCS_CSCR_BSTR			(0x00000010)
#define FBCS_CSCR_BSTW			(0x00000008)

#define FBCS_CSCR_PS_16			(0x00000080)
#define FBCS_CSCR_PS_8			(0x00000040)
#define FBCS_CSCR_PS_32			(0x00000000)
#endif

#endif				/* __FLEXBUS_H */
