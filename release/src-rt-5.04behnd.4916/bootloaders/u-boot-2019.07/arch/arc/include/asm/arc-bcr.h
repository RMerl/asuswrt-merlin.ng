/*
 * ARC Build Configuration Registers, with encoded hardware config
 *
 * Copyright (C) 2018 Synopsys
 * Author: Eugeniy Paltsev <Eugeniy.Paltsev@synopsys.com>
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2. This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#ifndef __ARC_BCR_H
#define __ARC_BCR_H
#ifndef __ASSEMBLY__

#include <config.h>

union bcr_di_cache {
	struct {
#ifdef CONFIG_CPU_BIG_ENDIAN
		unsigned int pad:12, line_len:4, sz:4, config:4, ver:8;
#else
		unsigned int ver:8, config:4, sz:4, line_len:4, pad:12;
#endif
	} fields;
	unsigned int word;
};

union bcr_slc_cfg {
	struct {
#ifdef CONFIG_CPU_BIG_ENDIAN
		unsigned int pad:24, way:2, lsz:2, sz:4;
#else
		unsigned int sz:4, lsz:2, way:2, pad:24;
#endif
	} fields;
	unsigned int word;
};

union bcr_generic {
	struct {
#ifdef CONFIG_CPU_BIG_ENDIAN
		unsigned int pad:24, ver:8;
#else
		unsigned int ver:8, pad:24;
#endif
	} fields;
	unsigned int word;
};

union bcr_clust_cfg {
	struct {
#ifdef CONFIG_CPU_BIG_ENDIAN
		unsigned int pad:7, c:1, num_entries:8, num_cores:8, ver:8;
#else
		unsigned int ver:8, num_cores:8, num_entries:8, c:1, pad:7;
#endif
	} fields;
	unsigned int word;
};

union bcr_mmu_4 {
	struct {
#ifdef CONFIG_CPU_BIG_ENDIAN
	unsigned int ver:8, sasid:1, sz1:4, sz0:4, res:2, pae:1,
		     n_ways:2, n_entry:2, n_super:2, u_itlb:3, u_dtlb:3;
#else
	/*           DTLB      ITLB      JES        JE         JA      */
	unsigned int u_dtlb:3, u_itlb:3, n_super:2, n_entry:2, n_ways:2,
		     pae:1, res:2, sz0:4, sz1:4, sasid:1, ver:8;
#endif
	} fields;
	unsigned int word;
};

#endif /* __ASSEMBLY__ */
#endif /* __ARC_BCR_H */
