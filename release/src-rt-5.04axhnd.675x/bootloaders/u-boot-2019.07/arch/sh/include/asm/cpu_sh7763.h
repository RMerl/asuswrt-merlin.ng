/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2008 Renesas Solutions Corp.
 * Copyright (C) 2007,2008 Nobuhiro Iwamatsu
 */
#ifndef _ASM_CPU_SH7763_H_
#define _ASM_CPU_SH7763_H_

/* CACHE */
#define CACHE_OC_NUM_WAYS	1
#define CCR				0xFF00001C
#define CCR_CACHE_INIT	0x0000090b

/* SCIF */
/* SCIF0 */
#define SCIF0_BASE	SCSMR0
#define SCSMR0		0xFFE00000

/* SCIF1 */
#define SCIF1_BASE	SCSMR1
#define SCSMR1		0xFFE08000

/* SCIF2 */
#define SCIF2_BASE	SCSMR2
#define SCSMR2		0xFFE10000

/* Watchdog Timer */
#define WTCNT		WDTST
#define WDTST		0xFFCC0000

/* TMU */
#define TMU_BASE	0xFFD80000

#endif /* _ASM_CPU_SH7763_H_ */
