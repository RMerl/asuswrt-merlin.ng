/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * ColdFire cache
 *
 * Copyright 2004-2012 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
 */

#ifndef __CACHE_H
#define __CACHE_H

#if defined(CONFIG_MCF520x) || defined(CONFIG_MCF523x) || \
    defined(CONFIG_MCF52x2) || defined(CONFIG_MCF5227x)
#define CONFIG_CF_V2
#endif

#if defined(CONFIG_MCF530x) || defined(CONFIG_MCF532x) || \
    defined(CONFIG_MCF5301x)
#define CONFIG_CF_V3
#endif

#if defined(CONFIG_MCF547x_8x) || defined(CONFIG_MCF5445x)
#define CONFIG_CF_V4
#elif defined(CONFIG_MCF5441x)
#define CONFIG_CF_V4E		/* Four Extra ACRn */
#endif

/* ***** CACR ***** */
/* V2 Core */
#ifdef CONFIG_CF_V2

#define CF_CACR_CENB		(1 << 31)
#define CF_CACR_CPD		(1 << 28)
#define CF_CACR_CFRZ		(1 << 27)
#define CF_CACR_CEIB		(1 << 10)
#define CF_CACR_DCM		(1 << 9)
#define CF_CACR_DBWE		(1 << 8)

#if defined(CONFIG_MCF5249) || defined(CONFIG_MCF5253)
#define CF_CACR_DWP		(1 << 6)
#else
#define CF_CACR_CINV		(1 << 24)
#define CF_CACR_DISI		(1 << 23)
#define CF_CACR_DISD		(1 << 22)
#define CF_CACR_INVI		(1 << 21)
#define CF_CACR_INVD		(1 << 20)
#define CF_CACR_DWP		(1 << 5)
#define CF_CACR_EUSP		(1 << 4)
#endif				/* CONFIG_MCF5249 || CONFIG_MCF5253 */

#endif				/* CONFIG_CF_V2 */

/* V3 Core */
#ifdef CONFIG_CF_V3

#define CF_CACR_EC		(1 << 31)
#define CF_CACR_ESB		(1 << 29)
#define CF_CACR_DPI		(1 << 28)
#define CF_CACR_HLCK		(1 << 27)
#define CF_CACR_CINVA		(1 << 24)
#define CF_CACR_DNFB		(1 << 10)
#define CF_CACR_DCM_UNMASK	0xFFFFFCFF
#define CF_CACR_DCM_WT		(0 << 8)
#define CF_CACR_DCM_CB		(1 << 8)
#define CF_CACR_DCM_P		(2 << 8)
#define CF_CACR_DCM_IP		(3 << 8)
#define CF_CACR_DW		(1 << 5)
#define CF_CACR_EUSP		(1 << 4)

#endif				/* CONFIG_CF_V3 */

/* V4 Core */
#if defined(CONFIG_CF_V4) || defined(CONFIG_CF_V4E)

#define CF_CACR_DEC		(1 << 31)
#define CF_CACR_DW		(1 << 30)
#define CF_CACR_DESB		(1 << 29)
#define CF_CACR_DDPI		(1 << 28)
#define CF_CACR_DHLCK		(1 << 27)
#define CF_CACR_DDCM_UNMASK	(0xF9FFFFFF)
#define CF_CACR_DDCM_WT		(0 << 25)
#define CF_CACR_DDCM_CB		(1 << 25)
#define CF_CACR_DDCM_P		(2 << 25)
#define CF_CACR_DDCM_IP		(3 << 25)
#define CF_CACR_DCINVA		(1 << 24)

#define CF_CACR_DDSP		(1 << 23)
#define CF_CACR_BEC		(1 << 19)
#define CF_CACR_BCINVA		(1 << 18)
#define CF_CACR_IEC		(1 << 15)
#define CF_CACR_DNFB		(1 << 13)
#define CF_CACR_IDPI		(1 << 12)
#define CF_CACR_IHLCK		(1 << 11)
#define CF_CACR_IDCM		(1 << 10)
#define CF_CACR_ICINVA		(1 << 8)
#define CF_CACR_IDSP		(1 << 7)
#define CF_CACR_EUSP		(1 << 5)

#if defined(CONFIG_MCF5445x) || defined(CONFIG_MCF5441x)
#define CF_CACR_IVO		(1 << 20)
#define CF_CACR_SPA		(1 << 14)
#else
#define CF_CACR_DF		(1 << 4)
#endif

#endif				/* CONFIG_CF_V4 */

/* ***** ACR ***** */
#define CF_ACR_ADR_UNMASK	(0x00FFFFFF)
#define CF_ACR_ADR(x)		((x & 0xFF) << 24)
#define CF_ACR_ADRMSK_UNMASK	(0xFF00FFFF)
#define CF_ACR_ADRMSK(x)	((x & 0xFF) << 16)
#define CF_ACR_EN		(1 << 15)
#define CF_ACR_SM_UNMASK	(0xFFFF9FFF)
#define CF_ACR_SM_UM		(0 << 13)
#define CF_ACR_SM_SM		(1 << 13)
#define CF_ACR_SM_ALL		(3 << 13)
#define CF_ACR_WP		(1 << 2)

/* V2 Core */
#ifdef CONFIG_CF_V2
#define CF_ACR_CM		(1 << 6)
#define CF_ACR_BWE		(1 << 5)
#else
/* V3 & V4 */
#define CF_ACR_CM_UNMASK	(0xFFFFFF9F)
#define CF_ACR_CM_WT		(0 << 5)
#define CF_ACR_CM_CB		(1 << 5)
#define CF_ACR_CM_P		(2 << 5)
#define CF_ACR_CM_IP		(3 << 5)
#endif				/* CONFIG_CF_V2 */

/* V4 Core */
#if defined(CONFIG_CF_V4) || defined(CONFIG_CF_V4E)
#define CF_ACR_AMM		(1 << 10)
#define CF_ACR_SP		(1 << 3)
#endif				/* CONFIG_CF_V4 */


#ifndef CONFIG_SYS_CACHE_ICACR
#define CONFIG_SYS_CACHE_ICACR	0
#endif

#ifndef CONFIG_SYS_CACHE_DCACR
#ifdef CONFIG_SYS_CACHE_ICACR
#define CONFIG_SYS_CACHE_DCACR	CONFIG_SYS_CACHE_ICACR
#else
#define CONFIG_SYS_CACHE_DCACR	0
#endif
#endif

#ifndef CONFIG_SYS_CACHE_ACR0
#define CONFIG_SYS_CACHE_ACR0	0
#endif

#ifndef CONFIG_SYS_CACHE_ACR1
#define CONFIG_SYS_CACHE_ACR1	0
#endif

#ifndef CONFIG_SYS_CACHE_ACR2
#define CONFIG_SYS_CACHE_ACR2	0
#endif

#ifndef CONFIG_SYS_CACHE_ACR3
#define CONFIG_SYS_CACHE_ACR3	0
#endif

#ifndef CONFIG_SYS_CACHE_ACR4
#define CONFIG_SYS_CACHE_ACR4	0
#endif

#ifndef CONFIG_SYS_CACHE_ACR5
#define CONFIG_SYS_CACHE_ACR5	0
#endif

#ifndef CONFIG_SYS_CACHE_ACR6
#define CONFIG_SYS_CACHE_ACR6	0
#endif

#ifndef CONFIG_SYS_CACHE_ACR7
#define CONFIG_SYS_CACHE_ACR7	0
#endif

#define CF_ADDRMASK(x)		(((x > 0x10) ? ((x >> 4) - 1) : (x)) << 16)

#ifndef __ASSEMBLY__		/* put C only stuff in this section */

void icache_invalid(void);
void dcache_invalid(void);

#endif

/*
 * m68k uses 16 byte L1 data cache line sizes.  Use this for DMA buffer
 * alignment unless the board configuration has specified a new value.
 */
#ifdef CONFIG_SYS_CACHELINE_SIZE
#define ARCH_DMA_MINALIGN	CONFIG_SYS_CACHELINE_SIZE
#else
#define ARCH_DMA_MINALIGN	16
#endif

#endif				/* __CACHE_H */
