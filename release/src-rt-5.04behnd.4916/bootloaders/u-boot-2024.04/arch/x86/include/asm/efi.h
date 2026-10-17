/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright Google LLC
 */

#ifndef _ASM_EFI_H_
#define _ASM_EFI_H_

struct efi_info;
struct screen_info;

/**
 * setup_video() - Set up the screen info in the x86 setup
 *
 * This is needed so Linux can use the display (when U-Boot is an EFI payload)
 *
 * @efi_info: Pointer to place to put the screen info in the x86 setup base
 */
void setup_video(struct screen_info *screen_info);

/**
 * setup_efi_info() - Set up the EFI info needed by Linux to boot
 *
 * This writes a suitable signature, table pointers, memory-map pointer, etc.
 * These are needed for Linux to boot from U-Boot (when U-Boot is an EFI
 * payload).
 *
 * @efi_info: Pointer to place to put the EFI info in the x86 setup base
 */
void setup_efi_info(struct efi_info *efi_info);

/**
 * efi_show_bdinfo() - Show information about EFI for the 'bdinfo' command
 *
 * This looks up the EFI table pointer and shows related info
 */
void efi_show_bdinfo(void);

#endif
