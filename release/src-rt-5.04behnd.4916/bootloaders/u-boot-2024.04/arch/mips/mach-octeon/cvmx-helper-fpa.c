// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018-2022 Marvell International Ltd.
 *
 * Helper functions for FPA setup.
 */

#include <errno.h>
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
#include <mach/cvmx-range.h>
#include <mach/cvmx-global-resources.h>

#include <mach/cvmx-agl-defs.h>
#include <mach/cvmx-bgxx-defs.h>
#include <mach/cvmx-ciu-defs.h>
#include <mach/cvmx-gmxx-defs.h>
#include <mach/cvmx-gserx-defs.h>
#include <mach/cvmx-ilk-defs.h>
#include <mach/cvmx-ipd-defs.h>
#include <mach/cvmx-pcsx-defs.h>
#include <mach/cvmx-pcsxx-defs.h>
#include <mach/cvmx-pki-defs.h>
#include <mach/cvmx-pko-defs.h>
#include <mach/cvmx-xcv-defs.h>

#include <mach/cvmx-hwpko.h>
#include <mach/cvmx-ilk.h>
#include <mach/cvmx-ipd.h>
#include <mach/cvmx-pki.h>
#include <mach/cvmx-pko3.h>
#include <mach/cvmx-pko3-queue.h>
#include <mach/cvmx-pko3-resources.h>
#include <mach/cvmx-pip.h>

#include <mach/cvmx-helper.h>
#include <mach/cvmx-helper-board.h>
#include <mach/cvmx-helper-cfg.h>

#include <mach/cvmx-helper-bgx.h>
#include <mach/cvmx-helper-cfg.h>
#include <mach/cvmx-helper-util.h>
#include <mach/cvmx-helper-pki.h>
#include <mach/cvmx-helper-pko.h>

/**
 * @INTERNAL
 * OBSOLETE
 *
 * Allocate memory for and initialize a single FPA pool.
 *
 * @param pool    Pool to initialize
 * @param buffer_size  Size of buffers to allocate in bytes
 * @param buffers Number of buffers to put in the pool. Zero is allowed
 * @param name    String name of the pool for debugging purposes
 * @return Zero on success, non-zero on failure
 *
 * This function is only for transition, will be removed.
 */
int __cvmx_helper_initialize_fpa_pool(int pool, u64 buffer_size, u64 buffers,
				      const char *name)
{
	return cvmx_fpa_setup_pool(pool, name, NULL, buffer_size, buffers);
}
