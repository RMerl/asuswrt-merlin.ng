/* SPDX-License-Identifier: GPL-2.0+ */
/*
 ********************************************************************
 * NOTE: This header file defines an interface to U-Boot. Including
 * this (unmodified) header file in another file is considered normal
 * use of U-Boot, and does *not* fall under the heading of "derived
 * work".
 ********************************************************************
 */

#ifndef __ASM_SH_U_BOOT_H_
#define __ASM_SH_U_BOOT_H_

/* Use the generic board which requires a unified bd_info */
#include <asm-generic/u-boot.h>

/* For image.h:image_check_target_arch() */
#define IH_ARCH_DEFAULT IH_ARCH_SH

#endif
