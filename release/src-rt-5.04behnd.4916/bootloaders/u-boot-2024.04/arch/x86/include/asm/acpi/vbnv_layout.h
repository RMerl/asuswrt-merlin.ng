/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright 2020 Google LLC
 */

#ifndef __ASM_ACPI_VBNV_LAYOUT_H__
#define __ASM_ACPI_VBNV_LAYOUT_H__

#define VBOOT_VBNV_BLOCK_SIZE 16	/* Size of NV storage block in bytes */

/* Constants for NV storage, for use with ACPI */
#define HEADER_OFFSET			0
#define HEADER_MASK			0xc0
#define HEADER_SIGNATURE		0x40
#define HEADER_FIRMWARE_SETTINGS_RESET	0x20
#define HEADER_KERNEL_SETTINGS_RESET	0x10

#define BOOT_OFFSET			1
#define BOOT_DEBUG_RESET_MODE		0x80
#define BOOT_DISABLE_DEV_REQUEST	0x40
#define BOOT_DISPLAY_REQUEST		0x20
#define BOOT_TRY_B_COUNT_MASK		0x0f

#define RECOVERY_OFFSET			2
#define LOCALIZATION_OFFSET		3

#define DEV_FLAGS_OFFSET		4
#define DEV_BOOT_USB_MASK		0x01
#define DEV_BOOT_SIGNED_ONLY_MASK	0x02
#define DEV_ENABLE_UDC			0x40

#define MISC_FLAGS_OFFSET		8
#define MISC_FLAGS_BATTERY_CUTOFF_MASK	0x08

#define KERNEL_FIELD_OFFSET		11
#define CRC_OFFSET			15

#endif /* __ASM_ACPI_VBNV_LAYOUT_H__ */
