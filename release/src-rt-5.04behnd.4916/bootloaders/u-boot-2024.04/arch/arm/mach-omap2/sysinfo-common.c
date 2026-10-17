// SPDX-License-Identifier: GPL-2.0+
/*
 * System information routines for all OMAP based boards.
 *
 * (C) Copyright 2017 Linaro Ltd.
 * Sam Protsenko <semen.protsenko@linaro.org>
 */

#include <asm/arch/omap.h>
#include <asm/io.h>
#include <asm/omap_common.h>

/**
 * Tell if device is GP/HS/EMU/TST.
 */
u32 get_device_type(void)
{
#if defined(CONFIG_OMAP34XX)
	/*
	 * On OMAP3 systems we call this early enough that we must just
	 * use the direct offset for safety.
	 */
	return (readl(OMAP34XX_CTRL_BASE + 0x2f0) & DEVICE_TYPE_MASK) >>
		DEVICE_TYPE_SHIFT;
#else
	return (readl((*ctrl)->control_status) & DEVICE_TYPE_MASK) >>
		DEVICE_TYPE_SHIFT;
#endif
}
