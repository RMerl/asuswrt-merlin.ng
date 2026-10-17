/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2000 - 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 ********************************************************************
 * NOTE: This header file defines an interface to U-Boot. Including
 * this (unmodified) header file in another file is considered normal
 * use of U-Boot, and does *not* fall under the heading of "derived
 * work".
 ********************************************************************
 */

#ifndef __U_BOOT_H__
#define __U_BOOT_H__

#include <config.h>

/* For image.h:image_check_target_arch() */
#define IH_ARCH_DEFAULT IH_ARCH_PPC

/* Use the generic board which requires a unified bd_info */
#include <asm-generic/u-boot.h>
#include <asm/ppc.h>
#include <asm/u-boot-ppc.h>

#endif	/* __U_BOOT_H__ */
