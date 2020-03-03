/*
 *	CPU feature overrides for DECstation systems.  Two variations
 *	are generally applicable.
 *
 *	Copyright (C) 2013  Maciej W. Rozycki
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 */
#ifndef __ASM_MACH_DEC_CPU_FEATURE_OVERRIDES_H
#define __ASM_MACH_DEC_CPU_FEATURE_OVERRIDES_H

/* Generic ones first.  */
#define cpu_has_tlb			1
#define cpu_has_tx39_cache		0
#define cpu_has_divec			0
#define cpu_has_prefetch		0
#define cpu_has_mcheck			0
#define cpu_has_ejtag			0
#define cpu_has_mips16			0
#define cpu_has_mdmx			0
#define cpu_has_mips3d			0
#define cpu_has_smartmips		0
#define cpu_has_rixi			0
#define cpu_has_vtag_icache		0
#define cpu_has_ic_fills_f_dc		0
#define cpu_has_pindexed_dcache		0
#define cpu_has_local_ebase		0
#define cpu_icache_snoops_remote_store	1
#define cpu_has_mips_4			0
#define cpu_has_mips_5			0
#define cpu_has_mips32r1		0
#define cpu_has_mips32r2		0
#define cpu_has_mips64r1		0
#define cpu_has_mips64r2		0
#define cpu_has_dsp			0
#define cpu_has_mipsmt			0
#define cpu_has_userlocal		0

/* R3k-specific ones.  */
#ifdef CONFIG_CPU_R3000
#define cpu_has_4kex			0
#define cpu_has_3k_cache		1
#define cpu_has_4k_cache		0
#define cpu_has_32fpr			0
#define cpu_has_counter			0
#define cpu_has_watch			0
#define cpu_has_vce			0
#define cpu_has_cache_cdex_p		0
#define cpu_has_cache_cdex_s		0
#define cpu_has_llsc			0
#define cpu_has_dc_aliases		0
#define cpu_has_mips_2			0
#define cpu_has_mips_3			0
#define cpu_has_nofpuex			1
#define cpu_has_inclusive_pcaches	0
#define cpu_dcache_line_size()		4
#define cpu_icache_line_size()		4
#define cpu_scache_line_size()		0
#endif /* CONFIG_CPU_R3000 */

/* R4k-specific ones.  */
#ifdef CONFIG_CPU_R4X00
#define cpu_has_4kex			1
#define cpu_has_3k_cache		0
#define cpu_has_4k_cache		1
#define cpu_has_32fpr			1
#define cpu_has_counter			1
#define cpu_has_watch			1
#define cpu_has_vce			1
#define cpu_has_cache_cdex_p		1
#define cpu_has_cache_cdex_s		1
#define cpu_has_llsc			1
#define cpu_has_dc_aliases		(PAGE_SIZE < 0x4000)
#define cpu_has_mips_2			1
#define cpu_has_mips_3			1
#define cpu_has_nofpuex			0
#define cpu_has_inclusive_pcaches	1
#define cpu_dcache_line_size()		16
#define cpu_icache_line_size()		16
#define cpu_scache_line_size()		32
#endif /* CONFIG_CPU_R4X00 */

#endif /* __ASM_MACH_DEC_CPU_FEATURE_OVERRIDES_H */
