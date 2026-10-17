/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2010-2011 Texas Instruments, <www.ti.com>
 * Mansoor Ahamed <mansoor.ahamed@ti.com>
 *
 * Derived from work done by Rohit Choraria <rohitkc@ti.com> for omap3
 */
#ifndef __ASM_ARCH_ELM_H
#define __ASM_ARCH_ELM_H
/*
 * ELM Module Registers
 */

/* ELM registers bit fields */
#define ELM_SYSCONFIG_SOFTRESET_MASK			(0x2)
#define ELM_SYSCONFIG_SOFTRESET			(0x2)
#define ELM_SYSSTATUS_RESETDONE_MASK			(0x1)
#define ELM_SYSSTATUS_RESETDONE			(0x1)
#define ELM_LOCATION_CONFIG_ECC_BCH_LEVEL_MASK		(0x3)
#define ELM_LOCATION_CONFIG_ECC_SIZE_MASK		(0x7FF0000)
#define ELM_LOCATION_CONFIG_ECC_SIZE_POS		(16)
#define ELM_SYNDROME_FRAGMENT_6_SYNDROME_VALID		(0x00010000)
#define ELM_LOCATION_STATUS_ECC_CORRECTABLE_MASK	(0x100)
#define ELM_LOCATION_STATUS_ECC_NB_ERRORS_MASK		(0x1F)

#define ELM_MAX_CHANNELS				8
#define ELM_MAX_ERROR_COUNT				16

#ifndef __ASSEMBLY__

enum bch_level {
	BCH_4_BIT = 0,
	BCH_8_BIT,
	BCH_16_BIT
};


/* BCH syndrome registers */
struct syndrome {
	u32 syndrome_fragment_x[7];	/* 0x400, 0x404.... 0x418 */
	u8 res1[36];			/* 0x41c */
};

/* BCH error status & location register */
struct location {
	u32 location_status;		/* 0x800 */
	u8 res1[124];			/* 0x804 */
	u32 error_location_x[ELM_MAX_ERROR_COUNT]; /* 0x880, 0x980, .. */
	u8 res2[64];			/* 0x8c0 */
};

/* BCH ELM register map - do not try to allocate memmory for this structure.
 * We have used plenty of reserved variables to fill the slots in the ELM
 * register memory map.
 * Directly initialize the struct pointer to ELM base address.
 */
struct elm {
	u32 rev;				/* 0x000 */
	u8 res1[12];				/* 0x004 */
	u32 sysconfig;				/* 0x010 */
	u32 sysstatus;				/* 0x014 */
	u32 irqstatus;				/* 0x018 */
	u32 irqenable;				/* 0x01c */
	u32 location_config;			/* 0x020 */
	u8 res2[92];				/* 0x024 */
	u32 page_ctrl;				/* 0x080 */
	u8 res3[892];				/* 0x084 */
	struct  syndrome syndrome_fragments[ELM_MAX_CHANNELS]; /* 0x400,0x420 */
	u8 res4[512];				/* 0x600 */
	struct location  error_location[ELM_MAX_CHANNELS]; /* 0x800,0x900 ... */
};

int elm_check_error(u8 *syndrome, enum bch_level bch_type, u32 *error_count,
		u32 *error_locations);
int elm_config(enum bch_level level);
void elm_reset(void);
void elm_init(void);
#endif /* __ASSEMBLY__ */
#endif /* __ASM_ARCH_ELM_H */
