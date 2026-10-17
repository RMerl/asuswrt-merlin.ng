/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2014 CompuLab, Ltd. <www.compulab.co.il>
 *
 * Authors: Igor Grinberg <grinberg@compulab.co.il>
 */

#ifndef _CL_COMMON_
#define _CL_COMMON_

#include <linux/errno.h>

void cl_print_pcb_info(void);

#ifdef CONFIG_CMD_USB
int cl_usb_hub_init(int gpio, const char *label);
void cl_usb_hub_deinit(int gpio);
#else /* !CONFIG_CMD_USB */
static inline int cl_usb_hub_init(int gpio, const char *label)
{
	return -ENOSYS;
}
static inline void cl_usb_hub_deinit(int gpio) {}
#endif /* CONFIG_CMD_USB */

#ifdef CONFIG_SMC911X
int cl_omap3_smc911x_init(int id, int cs, u32 base_addr,
			  int (*reset)(int), int rst_gpio);
#else /* !CONFIG_SMC911X */
static inline int cl_omap3_smc911x_init(int id, int cs, u32 base_addr,
					int (*reset)(int), int rst_gpio)
{
	return -ENOSYS;
}
#endif /* CONFIG_SMC911X */

#endif /* _CL_COMMON_ */
