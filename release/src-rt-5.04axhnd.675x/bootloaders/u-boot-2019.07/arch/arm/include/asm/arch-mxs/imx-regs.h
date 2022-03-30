/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Freescale i.MX23/i.MX28 Registers
 *
 * Copyright (C) 2011 Marek Vasut <marek.vasut@gmail.com>
 * on behalf of DENX Software Engineering GmbH
 */

#ifndef __IMX_REGS_H__
#define __IMX_REGS_H__

#include <asm/mach-imx/regs-apbh.h>
#include <asm/arch/regs-base.h>
#include <asm/mach-imx/regs-bch.h>
#include <asm/arch/regs-digctl.h>
#include <asm/mach-imx/regs-gpmi.h>
#include <asm/mach-imx/regs-lcdif.h>
#include <asm/arch/regs-i2c.h>
#include <asm/arch/regs-lradc.h>
#include <asm/arch/regs-ocotp.h>
#include <asm/arch/regs-pinctrl.h>
#include <asm/arch/regs-rtc.h>
#include <asm/arch/regs-ssp.h>
#include <asm/arch/regs-timrot.h>
#include <asm/arch/regs-usb.h>
#include <asm/arch/regs-usbphy.h>

#ifdef CONFIG_MX23
#include <asm/arch/regs-clkctrl-mx23.h>
#include <asm/arch/regs-power-mx23.h>
#endif

#ifdef CONFIG_MX28
#include <asm/arch/regs-clkctrl-mx28.h>
#include <asm/arch/regs-power-mx28.h>
#endif

#endif	/* __IMX_REGS_H__ */
