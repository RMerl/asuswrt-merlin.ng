/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Common internal memory map for some Freescale SoCs
 *
 * Copyright 2015 Freescale Semiconductor, Inc.
 */

#ifndef __FSL_SEC_MON_H
#define __FSL_SEC_MON_H

#include <common.h>
#include <asm/io.h>

#ifdef CONFIG_SYS_FSL_SEC_MON_LE
#define sec_mon_in32(a)       in_le32(a)
#define sec_mon_out32(a, v)   out_le32(a, v)
#define sec_mon_in16(a)       in_le16(a)
#define sec_mon_clrbits32     clrbits_le32
#define sec_mon_setbits32     setbits_le32
#elif defined(CONFIG_SYS_FSL_SEC_MON_BE)
#define sec_mon_in32(a)       in_be32(a)
#define sec_mon_out32(a, v)   out_be32(a, v)
#define sec_mon_in16(a)       in_be16(a)
#define sec_mon_clrbits32     clrbits_be32
#define sec_mon_setbits32     setbits_be32
#else
#error Neither CONFIG_SYS_FSL_SEC_MON_LE nor CONFIG_SYS_FSL_SEC_MON_BE defined
#endif

struct ccsr_sec_mon_regs {
	u8 reserved0[0x04];
	u32 hp_com;	/* 0x04 SEC_MON_HP Command Register */
	u8 reserved2[0x0c];
	u32 hp_stat;	/* 0x08 SEC_MON_HP Status Register */
};

#define HPCOMR_SW_SV		0x100	/* Security Violation bit */
#define HPCOMR_SW_FSV		0x200	/* Fatal Security Violation bit */
#define HPCOMR_SSM_ST		0x1	/* SSM_ST field in SEC_MON command */
#define HPCOMR_SSM_ST_DIS	0x2	/* Disable Secure to Trusted State */
#define HPCOMR_SSM_SFNS_DIS	0x4	/* Disable Soft Fail to Non-Secure */
#define HPSR_SSM_ST_CHECK	0x900	/* SEC_MON is in check state */
#define HPSR_SSM_ST_NON_SECURE	0xb00	/* SEC_MON is in non secure state */
#define HPSR_SSM_ST_TRUST	0xd00	/* SEC_MON is in trusted state */
#define HPSR_SSM_ST_SOFT_FAIL	0x300	/* SEC_MON is in soft fail state */
#define HPSR_SSM_ST_SECURE	0xf00	/* SEC_MON is in secure state */
#define HPSR_SSM_ST_MASK	0xf00	/* Mask for SSM_ST field */

/*
 * SEC_MON read. This specifies the possible reads
 * from the SEC_MON
 */
enum {
	SEC_MON_SSM_ST,
	SEC_MON_SW_FSV,
	SEC_MON_SW_SV,
};

/* Transition SEC_MON state */
int set_sec_mon_state(u32 state);

#endif /* __FSL_SEC_MON_H */
