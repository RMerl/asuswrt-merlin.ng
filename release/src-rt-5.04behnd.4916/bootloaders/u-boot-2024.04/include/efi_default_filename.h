/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * When a boot option does not provide a file path the EFI file to be
 * booted is \EFI\BOOT\$(BOOTEFI_NAME).EFI. The architecture specific
 * file name is defined in this include.
 *
 * Copyright (c) 2022, Heinrich Schuchardt <xypron.glpk@gmx.de>
 * Copyright (c) 2022, Linaro Limited
 */

#ifndef _EFI_DEFAULT_FILENAME_H
#define _EFI_DEFAULT_FILENAME_H

#include <host_arch.h>

#undef BOOTEFI_NAME

#ifdef CONFIG_SANDBOX

#if HOST_ARCH == HOST_ARCH_X86_64
#define BOOTEFI_NAME "BOOTX64.EFI"
#elif HOST_ARCH == HOST_ARCH_X86
#define BOOTEFI_NAME "BOOTIA32.EFI"
#elif HOST_ARCH == HOST_ARCH_AARCH64
#define BOOTEFI_NAME "BOOTAA64.EFI"
#elif HOST_ARCH == HOST_ARCH_ARM
#define BOOTEFI_NAME "BOOTARM.EFI"
#elif HOST_ARCH == HOST_ARCH_RISCV32
#define BOOTEFI_NAME "BOOTRISCV32.EFI"
#elif HOST_ARCH == HOST_ARCH_RISCV64
#define BOOTEFI_NAME "BOOTRISCV64.EFI"
#else
#error Unsupported UEFI architecture
#endif

#else

#if defined(CONFIG_ARM64)
#define BOOTEFI_NAME "BOOTAA64.EFI"
#elif defined(CONFIG_ARM)
#define BOOTEFI_NAME "BOOTARM.EFI"
#elif defined(CONFIG_X86_64)
#define BOOTEFI_NAME "BOOTX64.EFI"
#elif defined(CONFIG_X86)
#define BOOTEFI_NAME "BOOTIA32.EFI"
#elif defined(CONFIG_ARCH_RV32I)
#define BOOTEFI_NAME "BOOTRISCV32.EFI"
#elif defined(CONFIG_ARCH_RV64I)
#define BOOTEFI_NAME "BOOTRISCV64.EFI"
#else
#error Unsupported UEFI architecture
#endif

#endif

#endif
