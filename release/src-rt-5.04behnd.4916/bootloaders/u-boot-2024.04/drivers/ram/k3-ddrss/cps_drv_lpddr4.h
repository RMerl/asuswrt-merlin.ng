/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Cadence DDR Driver
 *
 * Copyright (C) 2012-2022 Cadence Design Systems, Inc.
 * Copyright (C) 2018-2022 Texas Instruments Incorporated - https://www.ti.com/
 */

#ifndef CPS_DRV_H_
#define CPS_DRV_H_

#ifdef DEMO_TB
#include <cdn_demo.h>
#else
#include <asm/io.h>
#endif

#define CPS_REG_READ(reg) (cps_regread((volatile u32 *)(reg)))

#define CPS_REG_WRITE(reg, value) (cps_regwrite((volatile u32 *)(reg), (u32)(value)))

#define CPS_FLD_MASK(fld)  (fld ## _MASK)
#define CPS_FLD_SHIFT(fld) (fld ## _SHIFT)
#define CPS_FLD_WIDTH(fld) (fld ## _WIDTH)
#define CPS_FLD_WOCLR(fld) (fld ## _WOCLR)
#define CPS_FLD_WOSET(fld) (fld ## _WOSET)

#define CPS_FLD_READ(fld, reg_value) (cps_fldread((u32)(CPS_FLD_MASK(fld)),  \
						  (u32)(CPS_FLD_SHIFT(fld)), \
						  (u32)(reg_value)))

#define CPS_FLD_WRITE(fld, reg_value, value) (cps_fldwrite((u32)(CPS_FLD_MASK(fld)),  \
							   (u32)(CPS_FLD_SHIFT(fld)), \
							   (u32)(reg_value), (u32)(value)))

#define CPS_FLD_SET(fld, reg_value) (cps_fldset((u32)(CPS_FLD_WIDTH(fld)), \
						(u32)(CPS_FLD_MASK(fld)),  \
						(u32)(CPS_FLD_WOCLR(fld)), \
						(u32)(reg_value)))

#ifdef CLR_USED
#define CPS_FLD_CLEAR(reg, fld, reg_value) (cps_fldclear((u32)(CPS_FLD_WIDTH(fld)), \
							 (u32)(CPS_FLD_MASK(fld)),  \
							 (u32)(CPS_FLD_WOSET(fld)), \
							 (u32)(CPS_FLD_WOCLR(fld)), \
							 (u32)(reg_value)))

#endif
static inline u32 cps_regread(volatile u32 *reg);
static inline u32 cps_regread(volatile u32 *reg)
{
	return readl(reg);
}

static inline void cps_regwrite(volatile u32 *reg, u32 value);
static inline void cps_regwrite(volatile u32 *reg, u32 value)
{
	writel(value, reg);
}

static inline u32 cps_fldread(u32 mask, u32 shift, u32 reg_value);
static inline u32 cps_fldread(u32 mask, u32 shift, u32 reg_value)
{
	u32 result = (reg_value & mask) >> shift;

	return result;
}

static inline u32 cps_fldwrite(u32 mask, u32 shift, u32 reg_value, u32 value);
static inline u32 cps_fldwrite(u32 mask, u32 shift, u32 reg_value, u32 value)
{
	u32 new_value = (value << shift) & mask;

	new_value = (reg_value & ~mask) | new_value;
	return new_value;
}

static inline u32 cps_fldset(u32 width, u32 mask, u32 is_woclr, u32 reg_value);
static inline u32 cps_fldset(u32 width, u32 mask, u32 is_woclr, u32 reg_value)
{
	u32 new_value = reg_value;

	if ((width == 1U) && (is_woclr == 0U))
		new_value |= mask;

	return new_value;
}

#ifdef CLR_USED
static inline u32 cps_fldclear(u32 width, u32 mask, u32 is_woset, u32 is_woclr, u32 reg_value);
static inline u32 cps_fldclear(u32 width, u32 mask, u32 is_woset, u32 is_woclr, u32 reg_value)
{
	u32 new_value = reg_value;

	if ((width == 1U) && (is_woset == 0U))
		new_value = (new_value & ~mask) | ((is_woclr != 0U) ? mask : 0U);

	return new_value;
}
#endif /* CLR_USED */

#endif /* CPS_DRV_H_ */
