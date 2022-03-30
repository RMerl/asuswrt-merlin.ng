/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2016 Google, Inc
 */

#ifndef __asm_arch_rcba_h
#define __asm_arch_rcba_h

#define ACPIIRQEN	0x31e0	/* 32bit */

#define PMSYNC_CONFIG	0x33c4	/* 32bit */
#define PMSYNC_CONFIG2	0x33cc	/* 32bit */

#define DEEP_S3_POL	0x3328	/* 32bit */
#define  DEEP_S3_EN_AC		(1 << 0)
#define  DEEP_S3_EN_DC		(1 << 1)
#define DEEP_S5_POL	0x3330	/* 32bit */
#define  DEEP_S5_EN_AC		(1 << 14)
#define  DEEP_S5_EN_DC		(1 << 15)
#define DEEP_SX_CONFIG	0x3334	/* 32bit */
#define  DEEP_SX_WAKE_PIN_EN	(1 << 2)
#define  DEEP_SX_ACPRESENT_PD	(1 << 1)
#define  DEEP_SX_GP27_PIN_EN	(1 << 0)
#define PMSYNC_CONFIG	0x33c4	/* 32bit */
#define PMSYNC_CONFIG2	0x33cc	/* 32bit */

#define RC		0x3400	/* 32bit */
#define HPTC		0x3404	/* 32bit */
#define GCS		0x3410	/* 32bit */
#define BUC		0x3414	/* 32bit */
#define PCH_DISABLE_GBE		(1 << 5)
#define FD		0x3418	/* 32bit */
#define FDSW		0x3420	/* 8bit */
#define DISPBDF		0x3424  /* 16bit */
#define FD2		0x3428	/* 32bit */
#define CG		0x341c	/* 32bit */

/* Function Disable 1 RCBA 0x3418 */
#define PCH_DISABLE_ALWAYS	(1 << 0)
#define PCH_DISABLE_ADSPD	(1 << 1)
#define PCH_DISABLE_SATA1	(1 << 2)
#define PCH_DISABLE_SMBUS	(1 << 3)
#define PCH_DISABLE_HD_AUDIO	(1 << 4)
#define PCH_DISABLE_EHCI2	(1 << 13)
#define PCH_DISABLE_LPC		(1 << 14)
#define PCH_DISABLE_EHCI1	(1 << 15)
#define PCH_DISABLE_PCIE(x)	(1 << (16 + x))
#define PCH_DISABLE_THERMAL	(1 << 24)
#define PCH_DISABLE_SATA2	(1 << 25)
#define PCH_DISABLE_XHCI	(1 << 27)

/* Function Disable 2 RCBA 0x3428 */
#define PCH_DISABLE_KT		(1 << 4)
#define PCH_DISABLE_IDER	(1 << 3)
#define PCH_DISABLE_MEI2	(1 << 2)
#define PCH_DISABLE_MEI1	(1 << 1)
#define PCH_ENABLE_DBDF		(1 << 0)

#endif
