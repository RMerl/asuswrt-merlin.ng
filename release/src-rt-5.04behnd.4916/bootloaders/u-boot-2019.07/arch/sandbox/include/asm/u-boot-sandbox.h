/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2011 The Chromium OS Authors.
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Alex Zuepke <azu@sysgo.de>
 */

#ifndef _U_BOOT_SANDBOX_H_
#define _U_BOOT_SANDBOX_H_

/* board/.../... */
int board_init(void);

/* start.c */
int sandbox_early_getopt_check(void);
int sandbox_main_loop_init(void);

int cleanup_before_linux(void);

/* drivers/video/sandbox_sdl.c */
int sandbox_lcd_sdl_early_init(void);

/**
 * pci_map_physmem() - map a PCI device into memory
 *
 * This is used on sandbox to map a device into memory so that it can be
 * used with normal memory access. After this call, some part of the device's
 * internal structure becomes visible.
 *
 * This function is normally called from sandbox's map_sysmem() automatically.
 *
 * @paddr:	Physical memory address, normally corresponding to a PCI BAR
 * @lenp:	On entry, the size of the area to map, On exit it is updated
 *		to the size actually mapped, which may be less if the device
 *		has less space
 * @devp:	Returns the device which mapped into this space
 * @ptrp:	Returns a pointer to the mapped address. The device's space
 *		can be accessed as @lenp bytes starting here
 * @return 0 if OK, -ve on error
 */
int pci_map_physmem(phys_addr_t paddr, unsigned long *lenp,
		    struct udevice **devp, void **ptrp);

/**
 * pci_unmap_physmem() - undo a memory mapping
 *
 * This must be called after pci_map_physmem() to undo the mapping.
 *
 * @paddr:	Physical memory address, as passed to pci_map_physmem()
 * @len:	Size of area mapped, as returned by pci_map_physmem()
 * @dev:	Device to unmap, as returned by pci_map_physmem()
 * @return 0 if OK, -ve on error
 */
int pci_unmap_physmem(const void *addr, unsigned long len,
		      struct udevice *dev);

/**
 * sandbox_set_enable_pci_map() - Enable / disable PCI address mapping
 *
 * Since address mapping involves calling every driver, provide a way to
 * enable and disable this. It can be handled automatically by the emulator
 * uclass, which knows if any emulators are currently active.
 *
 * If this is disabled, pci_map_physmem() will not be called from
 * map_sysmem().
 *
 * @enable: 0 to disable, 1 to enable
 */
void sandbox_set_enable_pci_map(int enable);

/**
 * sandbox_read_fdt_from_file() - Read a device tree from a file
 *
 * Read a device tree file from a host file and set it up for use as the
 * control FDT.
 */
int sandbox_read_fdt_from_file(void);

/* Exit sandbox (quit U-Boot) */
void sandbox_exit(void);

#endif	/* _U_BOOT_SANDBOX_H_ */
