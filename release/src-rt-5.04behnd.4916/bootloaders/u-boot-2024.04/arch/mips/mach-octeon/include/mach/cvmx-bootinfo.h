/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 */

/*
 * Header file containing the ABI with the bootloader.
 */

#ifndef __CVMX_BOOTINFO_H__
#define __CVMX_BOOTINFO_H__

#include "cvmx-coremask.h"

/*
 * Current major and minor versions of the CVMX bootinfo block that is
 * passed from the bootloader to the application.  This is versioned
 * so that applications can properly handle multiple bootloader
 * versions.
 */
#define CVMX_BOOTINFO_MAJ_VER 1
#define CVMX_BOOTINFO_MIN_VER 4

#if (CVMX_BOOTINFO_MAJ_VER == 1)
#define CVMX_BOOTINFO_OCTEON_SERIAL_LEN 20
/*
 * This structure is populated by the bootloader.  For binary
 * compatibility the only changes that should be made are
 * adding members to the end of the structure, and the minor
 * version should be incremented at that time.
 * If an incompatible change is made, the major version
 * must be incremented, and the minor version should be reset
 * to 0.
 */
struct cvmx_bootinfo {
	u32 major_version;
	u32 minor_version;

	u64 stack_top;
	u64 heap_base;
	u64 heap_end;
	u64 desc_vaddr;

	u32 exception_base_addr;
	u32 stack_size;
	u32 flags;
	u32 core_mask;
	/* DRAM size in megabytes */
	u32 dram_size;
	/* physical address of free memory descriptor block*/
	u32 phy_mem_desc_addr;
	/* used to pass flags from app to debugger */
	u32 debugger_flags_base_addr;

	/* CPU clock speed, in hz */
	u32 eclock_hz;

	/* DRAM clock speed, in hz */
	u32 dclock_hz;

	u32 reserved0;
	u16 board_type;
	u8 board_rev_major;
	u8 board_rev_minor;
	u16 reserved1;
	u8 reserved2;
	u8 reserved3;
	char board_serial_number[CVMX_BOOTINFO_OCTEON_SERIAL_LEN];
	u8 mac_addr_base[6];
	u8 mac_addr_count;
#if (CVMX_BOOTINFO_MIN_VER >= 1)
	/*
	 * Several boards support compact flash on the Octeon boot
	 * bus.	 The CF memory spaces may be mapped to different
	 * addresses on different boards.  These are the physical
	 * addresses, so care must be taken to use the correct
	 * XKPHYS/KSEG0 addressing depending on the application's
	 * ABI.	 These values will be 0 if CF is not present.
	 */
	u64 compact_flash_common_base_addr;
	u64 compact_flash_attribute_base_addr;
	/*
	 * Base address of the LED display (as on EBT3000 board)
	 * This will be 0 if LED display not present.
	 */
	u64 led_display_base_addr;
#endif
#if (CVMX_BOOTINFO_MIN_VER >= 2)
	/* DFA reference clock in hz (if applicable)*/
	u32 dfa_ref_clock_hz;

	/*
	 * flags indicating various configuration options.  These
	 * flags supercede the 'flags' variable and should be used
	 * instead if available.
	 */
	u32 config_flags;
#endif
#if (CVMX_BOOTINFO_MIN_VER >= 3)
	/*
	 * Address of the OF Flattened Device Tree structure
	 * describing the board.
	 */
	u64 fdt_addr;
#endif
#if (CVMX_BOOTINFO_MIN_VER >= 4)
	/*
	 * Coremask used for processors with more than 32 cores
	 * or with OCI.  This replaces core_mask.
	 */
	struct cvmx_coremask ext_core_mask;
#endif
};

#define CVMX_BOOTINFO_CFG_FLAG_PCI_HOST			(1ull << 0)
#define CVMX_BOOTINFO_CFG_FLAG_PCI_TARGET		(1ull << 1)
#define CVMX_BOOTINFO_CFG_FLAG_DEBUG			(1ull << 2)
#define CVMX_BOOTINFO_CFG_FLAG_NO_MAGIC			(1ull << 3)
/*
 * This flag is set if the TLB mappings are not contained in the
 * 0x10000000 - 0x20000000 boot bus region.
 */
#define CVMX_BOOTINFO_CFG_FLAG_OVERSIZE_TLB_MAPPING	(1ull << 4)
#define CVMX_BOOTINFO_CFG_FLAG_BREAK			(1ull << 5)

#endif /*   (CVMX_BOOTINFO_MAJ_VER == 1) */

#endif /* __CVMX_BOOTINFO_H__ */
