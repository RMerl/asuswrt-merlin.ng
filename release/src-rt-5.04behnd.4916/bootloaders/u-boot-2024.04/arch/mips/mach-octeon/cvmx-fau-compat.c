// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018-2022 Marvell International Ltd.
 */

#include <log.h>
#include <time.h>
#include <linux/delay.h>

#include <mach/cvmx-regs.h>
#include <mach/cvmx-csr.h>
#include <mach/cvmx-bootmem.h>
#include <mach/octeon-model.h>
#include <mach/cvmx-fuse.h>
#include <mach/octeon-feature.h>
#include <mach/cvmx-qlm.h>
#include <mach/octeon_qlm.h>
#include <mach/cvmx-pcie.h>
#include <mach/cvmx-coremask.h>

#include <mach/cvmx-global-resources.h>

#include <mach/cvmx-pki.h>
#include <mach/cvmx-helper.h>
#include <mach/cvmx-helper-board.h>
#include <mach/cvmx-helper-cfg.h>

#include <mach/cvmx-hwfau.h>

u8 *cvmx_fau_regs_ptr;

void cvmx_fau_bootmem_init(void *bootmem)
{
	memset(bootmem, 0, CVMX_FAU_MAX_REGISTERS_8);
}

/**
 * Initializes FAU region for devices without FAU unit.
 * @return 0 on success -1 on failure
 */
int cvmx_fau_init(void)
{
	cvmx_fau_regs_ptr = (u8 *)cvmx_bootmem_alloc_named_range_once(
		CVMX_FAU_MAX_REGISTERS_8, 0, 1ull << 31, 128,
		"cvmx_fau_registers", cvmx_fau_bootmem_init);

	if (cvmx_fau_regs_ptr == 0ull) {
		debug("ERROR: Failed to alloc named block for software FAU.\n");
		return -1;
	}

	return 0;
}
