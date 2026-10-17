/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2007, Tensilica Inc.
 *
 ********************************************************************
 * NOTE: This header file defines an interface to U-Boot. Including
 * this (unmodified) header file in another file is considered normal
 * use of U-Boot, and does *not* fall under the heading of "derived
 * work".
 ********************************************************************
 */

#ifndef _XTENSA_U_BOOT_H
#define _XTENSA_U_BOOT_H

/* Use the generic board which requires a unified bd_info */
#include <asm-generic/u-boot.h>

/* For image.h:image_check_target_arch() */
#define IH_ARCH_DEFAULT IH_ARCH_XTENSA

#endif	/* _XTENSA_U_BOOT_H */
