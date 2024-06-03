/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2015, Freescale Semiconductor, Inc.
 */

#ifndef __ARCH_ARM_MACH_S32V234_MCRGM_REGS_H__
#define __ARCH_ARM_MACH_S32V234_MCRGM_REGS_H__

#define MC_RGM_DES			(MC_RGM_BASE_ADDR)
#define MC_RGM_FES			(MC_RGM_BASE_ADDR + 0x300)
#define MC_RGM_FERD			(MC_RGM_BASE_ADDR + 0x310)
#define MC_RGM_FBRE			(MC_RGM_BASE_ADDR + 0x330)
#define MC_RGM_FESS			(MC_RGM_BASE_ADDR + 0x340)
#define MC_RGM_DDR_HE			(MC_RGM_BASE_ADDR + 0x350)
#define MC_RGM_DDR_HS			(MC_RGM_BASE_ADDR + 0x354)
#define MC_RGM_FRHE			(MC_RGM_BASE_ADDR + 0x358)
#define MC_RGM_FREC			(MC_RGM_BASE_ADDR + 0x600)
#define MC_RGM_FRET			(MC_RGM_BASE_ADDR + 0x607)
#define MC_RGM_DRET			(MC_RGM_BASE_ADDR + 0x60B)

/* function reset sources mask */
#define F_SWT4				0x8000
#define F_JTAG				0x400
#define F_FCCU_SOFT			0x40
#define F_FCCU_HARD			0x20
#define F_SOFT_FUNC			0x8
#define F_ST_DONE			0x4
#define F_EXT_RST			0x1

#endif /* __ARCH_ARM_MACH_S32V234_MCRGM_REGS_H__ */
