/*
 * Copyright 2004, 2007 Freescale Semiconductor.
 * Copyright(c) 2003 Motorola Inc.
 */

#ifndef	__MPC85xx_H__
#define __MPC85xx_H__

#if defined(CONFIG_E500)
#include <e500.h>
#endif

/*
 * SCCR - System Clock Control Register, 9-8
 */
#define SCCR_CLPD       0x00000004      /* CPM Low Power Disable */
#define SCCR_DFBRG_MSK  0x00000003      /* Division by BRGCLK Mask */
#define SCCR_DFBRG_SHIFT 0

#define SCCR_DFBRG00    0x00000000      /* BRGCLK division by 4 */
#define SCCR_DFBRG01    0x00000001      /* BRGCLK div by 16 (normal) */
#define SCCR_DFBRG10    0x00000002      /* BRGCLK division by 64 */
#define SCCR_DFBRG11    0x00000003      /* BRGCLK division by 256 */

/*
 * Define default values for some CCSR macros to make header files cleaner*
 *
 * To completely disable CCSR relocation in a board header file, define
 * CONFIG_SPL_SYS_CCSR_DO_NOT_RELOCATE.  This will force CFG_SYS_CCSRBAR_PHYS
 * to a value that is the same as CFG_SYS_CCSRBAR.
 */

#ifdef CFG_SYS_CCSRBAR_PHYS
#error "Do not define CFG_SYS_CCSRBAR_PHYS directly.  Use \
CFG_SYS_CCSRBAR_PHYS_LOW and/or CFG_SYS_CCSRBAR_PHYS_HIGH instead."
#endif

#if CONFIG_IS_ENABLED(SYS_CCSR_DO_NOT_RELOCATE)
#undef CFG_SYS_CCSRBAR_PHYS_HIGH
#undef CFG_SYS_CCSRBAR_PHYS_LOW
#define CFG_SYS_CCSRBAR_PHYS_HIGH	0
#endif

#ifndef CFG_SYS_CCSRBAR
#define CFG_SYS_CCSRBAR		CONFIG_SYS_CCSRBAR_DEFAULT
#endif

#ifndef CFG_SYS_CCSRBAR_PHYS_HIGH
#ifdef CONFIG_PHYS_64BIT
#define CFG_SYS_CCSRBAR_PHYS_HIGH	0xf
#else
#define CFG_SYS_CCSRBAR_PHYS_HIGH	0
#endif
#endif

#ifndef CFG_SYS_CCSRBAR_PHYS_LOW
#define CFG_SYS_CCSRBAR_PHYS_LOW	CONFIG_SYS_CCSRBAR_DEFAULT
#endif

#define CFG_SYS_CCSRBAR_PHYS ((CFG_SYS_CCSRBAR_PHYS_HIGH * 1ull) << 32 | \
				 CFG_SYS_CCSRBAR_PHYS_LOW)

#endif	/* __MPC85xx_H__ */
