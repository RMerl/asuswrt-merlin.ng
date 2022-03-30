// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2013
 * Sergey Kostanbaev < sergey.kostanbaev <at> fairwaves.ru >
 */

#include <config.h>
#include <common.h>

#if defined(CONFIG_USB_OHCI_NEW) && defined(CONFIG_SYS_USB_OHCI_CPU_INIT)
#include <asm/io.h>
#include <asm/arch/ep93xx.h>

int usb_cpu_init(void)
{
	struct syscon_regs *syscon = (struct syscon_regs *)SYSCON_BASE;
	unsigned long pwr = readl(&syscon->pwrcnt);
	writel(pwr | SYSCON_PWRCNT_USH_EN, &syscon->pwrcnt);

	return 0;
}

int usb_cpu_stop(void)
{
	struct syscon_regs *syscon = (struct syscon_regs *)SYSCON_BASE;
	unsigned long pwr = readl(&syscon->pwrcnt);
	writel(pwr &  ~SYSCON_PWRCNT_USH_EN, &syscon->pwrcnt);

	return 0;
}

int usb_cpu_init_fail(void)
{
	return usb_cpu_stop();
}

#endif /* defined(CONFIG_USB_OHCI) && defined(CONFIG_SYS_USB_OHCI_CPU_INIT) */
