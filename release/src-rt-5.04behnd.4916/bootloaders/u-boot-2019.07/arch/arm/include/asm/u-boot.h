/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Alex Zuepke <azu@sysgo.de>
 *
 ********************************************************************
 * NOTE: This header file defines an interface to U-Boot. Including
 * this (unmodified) header file in another file is considered normal
 * use of U-Boot, and does *not* fall under the heading of "derived
 * work".
 ********************************************************************
 */

#ifndef _U_BOOT_H_
#define _U_BOOT_H_	1

/* Use the generic board which requires a unified bd_info */
#include <asm-generic/u-boot.h>
#include <asm/u-boot-arm.h>

/* For image.h:image_check_target_arch() */
#ifndef CONFIG_ARM64
#define IH_ARCH_DEFAULT IH_ARCH_ARM
#else
#define IH_ARCH_DEFAULT IH_ARCH_ARM64
#endif

#endif	/* _U_BOOT_H_ */
