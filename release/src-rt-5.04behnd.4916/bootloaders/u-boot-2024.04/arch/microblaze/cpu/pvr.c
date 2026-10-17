// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022, Ovidiu Panait <ovpanait@gmail.com>
 */
#include <asm/asm.h>
#include <asm/pvr.h>

int microblaze_cpu_has_pvr_full(void)
{
	u32 msr, pvr0;

	MFS(msr, rmsr);
	if (!(msr & PVR_MSR_BIT))
		return 0;

	get_pvr(0, pvr0);
	debug("%s: pvr0 is 0x%08x\n", __func__, pvr0);

	if (!(pvr0 & PVR0_PVR_FULL_MASK))
		return 0;

	return 1;
}

void microblaze_get_all_pvrs(u32 pvr[PVR_FULL_COUNT])
{
	get_pvr(0, pvr[0]);
	get_pvr(1, pvr[1]);
	get_pvr(2, pvr[2]);
	get_pvr(3, pvr[3]);
	get_pvr(4, pvr[4]);
	get_pvr(5, pvr[5]);
	get_pvr(6, pvr[6]);
	get_pvr(7, pvr[7]);
	get_pvr(8, pvr[8]);
	get_pvr(9, pvr[9]);
	get_pvr(10, pvr[10]);
	get_pvr(11, pvr[11]);
	get_pvr(12, pvr[12]);
}
