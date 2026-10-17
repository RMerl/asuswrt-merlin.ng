/* SPDX-License-Identifier:    GPL-2.0
 *
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * https://spdx.org/licenses
 */

#ifndef __SMC_H__
#define __SMC_H__

/* OcteonTX Service Calls version numbers */
#define OCTEONTX_VERSION_MAJOR	0x1
#define OCTEONTX_VERSION_MINOR	0x0

/* x1 - node number */
#define OCTEONTX_DRAM_SIZE	0xc2000301

ssize_t smc_dram_size(unsigned int node);

#endif /* __SMC_H__ */
