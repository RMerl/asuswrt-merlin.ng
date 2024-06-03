/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2009 Wind River Systems, Inc.
 * Tom Rix <Tom.Rix@windriver.com>
 *
 * This file is based on the file drivers/usb/musb/davinci.h
 *
 * This is the unique part of its copyright:
 *
 * --------------------------------------------------------------------
 *
 * Copyright (c) 2008 Texas Instruments
 * Author: Thomas Abraham t-abraham@ti.com, Texas Instruments
 *
 * --------------------------------------------------------------------
 */
#ifndef _MUSB_OMAP3_H_
#define _MUSB_OMAP3_H_

#include <asm/arch/cpu.h>
#include "musb_core.h"

/* Base address of MUSB registers */
#define MENTOR_USB0_BASE MUSB_BASE

/* Base address of OTG registers */
#define OMAP3_OTG_BASE (MENTOR_USB0_BASE + 0x400)

/* Timeout for USB module */
#define OMAP3_USB_TIMEOUT 0x3FFFFFF

int musb_platform_init(void);

#ifdef CONFIG_TARGET_OMAP3_EVM
extern u8 omap3_evm_need_extvbus(void);
#endif

#endif /* _MUSB_OMAP3_H */
