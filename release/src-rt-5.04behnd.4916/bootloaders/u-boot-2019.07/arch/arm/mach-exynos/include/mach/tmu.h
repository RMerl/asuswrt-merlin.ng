/*
 * Copyright (c) 2012 Samsung Electronics Co., Ltd.
 *      http://www.samsung.com
 * Akshay Saraswat <akshay.s@samsung.com>
 *
 * EXYNOS - Thermal Management Unit
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __ASM_ARCH_TMU_H
#define __ASM_ARCH_TMU_H

struct exynos5_tmu_reg {
	u32 triminfo;
	u32 rsvd1[4];
	u32 triminfo_control;
	u32 rsvd5[2];
	u32 tmu_control;
	u32 rsvd7;
	u32 tmu_status;
	u32 sampling_internal;
	u32 counter_value0;
	u32 counter_value1;
	u32 rsvd8[2];
	u32 current_temp;
	u32 rsvd10[3];
	u32 threshold_temp_rise;
	u32 threshold_temp_fall;
	u32 rsvd13[2];
	u32 past_temp3_0;
	u32 past_temp7_4;
	u32 past_temp11_8;
	u32 past_temp15_12;
	u32 inten;
	u32 intstat;
	u32 intclear;
	u32 rsvd15;
	u32 emul_con;
};
#endif /* __ASM_ARCH_TMU_H */
