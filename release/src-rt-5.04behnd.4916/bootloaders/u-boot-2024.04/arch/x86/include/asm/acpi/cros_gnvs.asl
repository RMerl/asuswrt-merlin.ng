/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright 2019 Google LLC
 */

/* This is the ChromeOS specific ACPI information needed by
 * the mainboard's chromeos.asl
 */

VBT0,   32,	// 0x000 - Boot Reason
VBT1,   32,	// 0x004 - Active Main Firmware
VBT2,   32,	// 0x008 - Active EC Firmware
VBT3,   16,	// 0x00c - CHSW
VBT4, 2048,	// 0x00e - HWID
VBT5,  512,	// 0x10e - FWID
VBT6,  512,	// 0x14e - FRID
VBT7,   32,	// 0x18e - active main firmware type
VBT8,   32,	// 0x192 - Recovery Reason
VBT9,   32,	// 0x196 - FMAP base address
CHVD, 24576,	// 0x19a - VDAT space filled by verified boot
VBTA,	32,	// 0xd9a - pointer to smbios FWID
MEHH,  256,	// 0xd9e - Management Engine Hash
RMOB,   32,	// 0xdbe - RAM oops base address
RMOL,   32,	// 0xdc2 - RAM oops length
ROVP,	32,	// 0xdc6 - pointer to RO_VPD
ROVL,	32,	// 0xdca - size of RO_VPD
RWVP,	32,	// 0xdce - pointer to RW_VPD
RWVL,	32,	// 0xdd2 - size of RW_VPD
		// 0xdd6
