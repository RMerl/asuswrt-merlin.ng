/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2015 Freescale Semiconductor, Inc.
 */

#ifndef _FSL_SFP_SNVS_
#define _FSL_SFP_SNVS_

#include <common.h>
#include <config.h>
#include <asm/io.h>

#ifdef CONFIG_SYS_FSL_SRK_LE
#define srk_in32(a)       in_le32(a)
#else
#define srk_in32(a)       in_be32(a)
#endif

#ifdef CONFIG_SYS_FSL_SFP_LE
#define sfp_in32(a)       in_le32(a)
#define sfp_out32(a, v)   out_le32(a, v)
#define sfp_in16(a)       in_le16(a)
#elif defined(CONFIG_SYS_FSL_SFP_BE)
#define sfp_in32(a)       in_be32(a)
#define sfp_out32(a, v)   out_be32(a, v)
#define sfp_in16(a)       in_be16(a)
#else
#error Neither CONFIG_SYS_FSL_SFP_LE nor CONFIG_SYS_FSL_SFP_BE is defined
#endif

/* Number of SRKH registers */
#define NUM_SRKH_REGS	8

#if	defined(CONFIG_SYS_FSL_SFP_VER_3_2) ||	\
	defined(CONFIG_SYS_FSL_SFP_VER_3_4)
struct ccsr_sfp_regs {
	u32 ospr;		/* 0x200 */
	u32 ospr1;		/* 0x204 */
	u32 reserved1[4];
	u32 fswpr;		/* 0x218 FSL Section Write Protect */
	u32 fsl_uid;		/* 0x21c FSL UID 0 */
	u32 fsl_uid_1;		/* 0x220 FSL UID 0 */
	u32 reserved2[12];
	u32 srk_hash[8];	/* 0x254 Super Root Key Hash */
	u32 oem_uid;		/* 0x274 OEM UID 0*/
	u32 oem_uid_1;		/* 0x278 OEM UID 1*/
	u32 oem_uid_2;		/* 0x27c OEM UID 2*/
	u32 oem_uid_3;		/* 0x280 OEM UID 3*/
	u32 oem_uid_4;		/* 0x284 OEM UID 4*/
	u32 reserved3[8];
};
#elif defined(CONFIG_SYS_FSL_SFP_VER_3_0)
struct ccsr_sfp_regs {
	u32 ospr;		/* 0x200 */
	u32 reserved0[14];
	u32 srk_hash[NUM_SRKH_REGS];	/* 0x23c Super Root Key Hash */
	u32 oem_uid;		/* 0x9c OEM Unique ID */
	u8 reserved2[0x04];
	u32 ovpr;			/* 0xA4  Intent To Secure */
	u8 reserved4[0x08];
	u32 fsl_uid;		/* 0xB0  FSL Unique ID */
	u8 reserved5[0x04];
	u32 fsl_spfr0;		/* Scratch Pad Fuse Register 0 */
	u32 fsl_spfr1;		/* Scratch Pad Fuse Register 1 */

};
#else
struct ccsr_sfp_regs {
	u8 reserved0[0x40];
	u32 ospr;	/* 0x40  OEM Security Policy Register */
	u8 reserved2[0x38];
	u32 srk_hash[8];	/* 0x7c  Super Root Key Hash */
	u32 oem_uid;	/* 0x9c  OEM Unique ID */
	u8 reserved4[0x4];
	u32 ovpr;	/* 0xA4  OEM Validation Policy Register */
	u8 reserved8[0x8];
	u32 fsl_uid;	/* 0xB0  FSL Unique ID */
};
#endif

#define ITS_MASK	0x00000004
#define ITS_BIT		2

#if defined(CONFIG_SYS_FSL_SFP_VER_3_4)
#define OSPR_KEY_REVOC_SHIFT    9
#define OSPR_KEY_REVOC_MASK     0x0000fe00
#else
#define OSPR_KEY_REVOC_SHIFT    13
#define OSPR_KEY_REVOC_MASK     0x0000e000
#endif /* CONFIG_SYS_FSL_SFP_VER_3_4 */

#endif
