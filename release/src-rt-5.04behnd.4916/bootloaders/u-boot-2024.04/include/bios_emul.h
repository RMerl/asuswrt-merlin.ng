/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 1996-1999 SciTech Software, Inc.
 */

#ifndef _BIOS_EMUL_H
#define _BIOS_EMUL_H

/* Include the register header directly here */
#include "../drivers/bios_emulator/include/x86emu/regs.h"
#include <pci.h>

/****************************************************************************
REMARKS:
Data structure used to describe the details for the BIOS emulator system
environment as used by the X86 emulator library.

HEADER:
biosemu.h

MEMBERS:
vgaInfo         - VGA BIOS information structure
biosmem_base    - Base of the BIOS image
biosmem_limit   - Limit of the BIOS image
busmem_base     - Base of the VGA bus memory
****************************************************************************/
typedef struct {
	int function;
	int device;
	int bus;
	u32 VendorID;
	u32 DeviceID;
	struct udevice *pcidev;
	void *BIOSImage;
	u32 BIOSImageLen;
	u8 LowMem[1536];
} BE_VGAInfo;

struct vesa_state;

int BootVideoCardBIOS(struct udevice *pcidev, BE_VGAInfo **pVGAInfo,
		      int clean_up);

/* Run a BIOS ROM natively (only supported on x86 machines) */
void bios_run_on_x86(struct udevice *dev, unsigned long addr, int vesa_mode,
		     struct vesa_state *mode_info);

/**
 * bios_set_interrupt_handler() - Install an interrupt handler for the BIOS
 *
 * This installs an interrupt handler that the BIOS will call when needed.
 *
 * @intnum:		Interrupt number to install a handler for
 * @int_handler_func:	Function to call to handle interrupt
 */
void bios_set_interrupt_handler(int intnum, int (*int_handler_func)(void));

void biosemu_set_interrupt_handler(int intnum, int (*int_func)(void));

int biosemu_setup(struct udevice *pcidev, BE_VGAInfo **pVGAInfo);

int biosemu_run(struct udevice *dev, uchar *bios_rom, int bios_len,
		BE_VGAInfo *vga_info, int clean_up, int vesa_mode,
		struct vesa_state *mode_info);

#endif
