/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2012-2014
 *     Texas Instruments Incorporated, <www.ti.com>
 */
#ifndef _PSC_DEFS_H_
#define _PSC_DEFS_H_

#include <asm/arch/hardware.h>

/*
 * FILE PURPOSE: Local Power Sleep Controller definitions
 *
 * FILE NAME: psc_defs.h
 *
 * DESCRIPTION: Provides local definitions for the power saver controller
 *
 */

/* Register offsets */
#define PSC_REG_PTCMD           0x120
#define PSC_REG_PSTAT	        0x128
#define PSC_REG_PDSTAT(x)       (0x200 + (4 * (x)))
#define PSC_REG_PDCTL(x)        (0x300 + (4 * (x)))
#define PSC_REG_MDCFG(x)        (0x600 + (4 * (x)))
#define PSC_REG_MDSTAT(x)       (0x800 + (4 * (x)))
#define PSC_REG_MDCTL(x)        (0xa00 + (4 * (x)))


static inline u32 _boot_bit_mask(u32 x, u32 y)
{
	u32 val = (1 << (x - y + 1)) - 1;
	return val << y;
}

static inline u32 boot_read_bitfield(u32 z, u32 x, u32 y)
{
	u32 val = z & _boot_bit_mask(x, y);
	return val >> y;
}

static inline u32 boot_set_bitfield(u32 z, u32 f, u32 x, u32 y)
{
	u32 mask = _boot_bit_mask(x, y);

	return (z & ~mask) | ((f << y) & mask);
}

/* PDCTL */
#define PSC_REG_PDCTL_SET_NEXT(x, y)        boot_set_bitfield((x), (y), 0, 0)
#define PSC_REG_PDCTL_SET_PDMODE(x, y)      boot_set_bitfield((x), (y), 15, 12)

/* PDSTAT */
#define PSC_REG_PDSTAT_GET_STATE(x)         boot_read_bitfield((x), 4, 0)

/* MDCFG */
#define PSC_REG_MDCFG_GET_PD(x)             boot_read_bitfield((x), 20, 16)
#define PSC_REG_MDCFG_GET_RESET_ISO(x)      boot_read_bitfield((x), 14, 14)

/* MDCTL */
#define PSC_REG_MDCTL_SET_NEXT(x, y)        boot_set_bitfield((x), (y), 4, 0)
#define PSC_REG_MDCTL_SET_LRSTZ(x, y)       boot_set_bitfield((x), (y), 8, 8)
#define PSC_REG_MDCTL_GET_LRSTZ(x)          boot_read_bitfield((x), 8, 8)
#define PSC_REG_MDCTL_SET_RESET_ISO(x, y)   boot_set_bitfield((x), (y), \
								  12, 12)

/* MDSTAT */
#define PSC_REG_MDSTAT_GET_STATUS(x)        boot_read_bitfield((x), 5, 0)
#define PSC_REG_MDSTAT_GET_LRSTZ(x)         boot_read_bitfield((x), 8, 8)
#define PSC_REG_MDSTAT_GET_LRSTDONE(x)      boot_read_bitfield((x), 9, 9)
#define PSC_REG_MDSTAT_GET_MRSTZ(x)         boot_read_bitfield((x), 10, 10)
#define PSC_REG_MDSTAT_GET_MRSTDONE(x)      boot_read_bitfield((x), 11, 11)

/* PDCTL states */
#define PSC_REG_VAL_PDCTL_NEXT_ON           1
#define PSC_REG_VAL_PDCTL_NEXT_OFF          0

#define PSC_REG_VAL_PDCTL_PDMODE_SLEEP      0

/* MDCTL states */
#define PSC_REG_VAL_MDCTL_NEXT_SWRSTDISABLE     0
#define PSC_REG_VAL_MDCTL_NEXT_OFF              2
#define PSC_REG_VAL_MDCTL_NEXT_ON               3

/* MDSTAT states */
#define PSC_REG_VAL_MDSTAT_STATE_ON             3
#define PSC_REG_VAL_MDSTAT_STATE_ENABLE_IN_PROG 0x24
#define PSC_REG_VAL_MDSTAT_STATE_OFF            2
#define PSC_REG_VAL_MDSTAT_STATE_DISABLE_IN_PROG1       0x20
#define PSC_REG_VAL_MDSTAT_STATE_DISABLE_IN_PROG2       0x21
#define PSC_REG_VAL_MDSTAT_STATE_DISABLE_IN_PROG3       0x22

/*
 * Timeout limit on checking PTSTAT. This is the number of times the
 * wait function will be called before giving up.
 */
#define PSC_PTSTAT_TIMEOUT_LIMIT    100000

u32 psc_get_domain_num(u32 mod_num);
int psc_enable_module(u32 mod_num);
int psc_disable_module(u32 mod_num);
int psc_disable_domain(u32 domain_num);
int psc_module_keep_in_reset_enabled(u32 mod_num, bool gate_clocks);
int psc_module_release_from_reset(u32 mod_num);

#endif /* _PSC_DEFS_H_ */
