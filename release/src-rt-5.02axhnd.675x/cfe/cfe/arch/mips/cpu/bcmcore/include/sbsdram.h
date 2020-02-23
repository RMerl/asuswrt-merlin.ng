/*
    Copyright 2001, Broadcom Corporation
    All Rights Reserved.
    
    This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
    the contents of this file may not be disclosed to third parties, copied or
    duplicated in any form, in whole or in part, without the prior written
    permission of Broadcom Corporation.
*/
/*
 * BCM47XX Sonics SiliconBackplane SDRAM controller core hardware definitions.
 *
 * $Id: sbsdram.h,v 1.1 2001/10/31 18:49:26 mpl Exp $
 */

#ifndef	_SBSDRAM_H
#define	_SBSDRAM_H

#ifndef _LANGUAGE_ASSEMBLY

/* Sonics side: SDRAM core registers */
typedef volatile struct sbsdramregs {
	uint32	initcontrol;	/* Generates external SDRAM initialization sequence */
	uint32	config;		/* Initializes external SDRAM mode register */
	uint32	refresh;	/* Controls external SDRAM refresh rate */
	uint32	pad1;
	uint32	pad2;
} sbsdramregs_t;

#endif

/* SDRAM initialization control (initcontrol) register bits */
#define SDRAM_CBR	0x0001	/* Writing 1 generates refresh cycle and toggles bit */
#define SDRAM_PRE	0x0002	/* Writing 1 generates precharge cycle and toggles bit */
#define SDRAM_MRS	0x0004	/* Writing 1 generates mode register select cycle and toggles bit */
#define SDRAM_EN	0x0008	/* When set, enables access to SDRAM */
#define SDRAM_16Mb	0x0000	/* Use 16 Megabit SDRAM */
#define SDRAM_64Mb	0x0010	/* Use 64 Megabit SDRAM */
#define SDRAM_128Mb	0x0020	/* Use 128 Megabit SDRAM */
#define SDRAM_RSVMb	0x0030	/* Use special SDRAM */
#define SDRAM_RST	0x0080	/* Writing 1 causes soft reset of controller */
#define SDRAM_SELFREF	0x0100	/* Writing 1 enables self refresh mode */
#define SDRAM_PWRDOWN	0x0200	/* Writing 1 causes controller to power down */
#define SDRAM_32BIT	0x0400	/* When set, indicates 32 bit SDRAM interface */
#define SDRAM_9BITCOL	0x0800	/* When set, indicates 9 bit column */

/* SDRAM configuration (config) register bits */
#define SDRAM_BURSTFULL	0x0000	/* Use full page bursts */
#define SDRAM_BURST8	0x0001	/* Use burst of 8 */
#define SDRAM_BURST4	0x0002	/* Use burst of 4 */
#define SDRAM_BURST2	0x0003	/* Use burst of 2 */
#define SDRAM_CAS3	0x0000	/* Use CAS latency of 3 */
#define SDRAM_CAS2	0x0004	/* Use CAS latency of 2 */

/* SDRAM refresh control (refresh) register bits */
#define SDRAM_REF(p)	(((p)&0xff) | SDRAM_REF_EN)	/* Refresh period */
#define SDRAM_REF_EN	0x8000		/* Writing 1 enables periodic refresh */

/* SDRAM Core Init values (OCP ID 0x803) */

#define SDRAM_CONFIG    SDRAM_BURSTFULL
#define SDRAM_REFRESH   SDRAM_REF(0x40)

#if	defined(MEM1MX16_KM)
#define	SDRAM_INIT	0x009
#elif	defined(MEM1MX16X2_KM)
#define SDRAM_INIT	0x409
#elif	defined(MEM2MX8X2_KM)
#define SDRAM_INIT	0x809
#elif	defined(MEM2MX8X4_KM)
#define SDRAM_INIT	0xc09
#elif	defined(MEM2MX32_KM)
#define SDRAM_INIT	0x439
#elif	defined(MEM4MX16_KM)
#define SDRAM_INIT	0x019
#elif	defined(MEM4MX16X2_KM)
#define SDRAM_INIT	0x419
#elif	defined(MEM8MX8X2_KM)
#define SDRAM_INIT	0x819
#elif	defined(MEM8MX8X4_KM)
#define SDRAM_INIT	0xc19
#elif	defined(MEM8MX16_KM)
#define SDRAM_INIT	0x829
#elif	defined(MEM8MX16X2_KM)
#define SDRAM_INIT	0xc29
#elif	defined(MEM4MX32_KM)
#define SDRAM_INIT	0x429
#else	/* DEFAULT MEM */
#define SDRAM_INIT	0x419
#endif

#endif	/* _SBSDRAM_H */
