/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2022, Ovidiu Panait <ovpanait@gmail.com>
 */

#ifndef __ASM_MICROBLAZE_CPUINFO_H
#define __ASM_MICROBLAZE_CPUINFO_H

#include <linux/types.h>

/**
 * struct microblaze_cpuinfo - CPU info for microblaze processor core.
 *
 * @icache_size: Size of instruction cache memory in bytes.
 * @icache_line_length: Instruction cache line length in bytes.
 * @dcache_size: Size of data cache memory in bytes.
 * @dcache_line_length: Data cache line length in bytes.
 * @use_mmu: MMU support flag.
 * @cpu_freq: Cpu clock frequency in Hz.
 * @addr_size: Address bus width in bits.
 * @ver_code: Cpu version code.
 * @fpga_code: FPGA family version code.
 */
struct microblaze_cpuinfo {
	u32 icache_size;
	u32 icache_line_length;

	u32 dcache_size;
	u32 dcache_line_length;

#if IS_ENABLED(CONFIG_CPU_MICROBLAZE)
	u32 use_mmu;
	u32 cpu_freq;
	u32 addr_size;

	u32 ver_code;
	u32 fpga_code;
#endif /* CONFIG_CPU_MICROBLAZE */
};

/**
 * struct microblaze_version_data - Maps a hex version code to a cpu/fpga name.
 */
struct microblaze_version_map {
	const char *string;
	const u32 code;
};

/**
 * microblaze_lookup_cpu_version_code() - Get hex version code for the
 * specified cpu name string.
 *
 * This function searches the cpu_ver_lookup[] array for the hex version code
 * associated with a specific CPU name. The version code is returned if a match
 * is found, otherwise 0.
 *
 * @string: cpu name string
 *
 * Return: >0 if the entry is found, 0 otherwise.
 */
const u32 microblaze_lookup_cpu_version_code(const char *string);

/**
 * microblaze_lookup_fpga_family_code() - Get hex version code for the
 * specified fpga family name.
 *
 * This function searches the family_string_lookup[] array for the hex version
 * code associated with a specific fpga family name. The version code is
 * returned if a match is found, otherwise 0.
 *
 * @string: fpga family name string
 *
 * Return: >0 if the entry is found, 0 otherwise.
 */
const u32 microblaze_lookup_fpga_family_code(const char *string);

/**
 * microblaze_lookup_cpu_version_string() - Get cpu name for the specified cpu
 * version code.
 *
 * This function searches the cpu_ver_lookup[] array for the cpu name string
 * associated with a specific version code. The cpu name is returned if a match
 * is found, otherwise "(unknown)".
 *
 * @code: cpu version code
 *
 * Return: Pointer to the cpu name if the entry is found, otherwise "(unknown)".
 */
const char *microblaze_lookup_cpu_version_string(const u32 code);

/**
 * microblaze_lookup_fpga_family_string() - Get fpga family name for the
 * specified version code.
 *
 * This function searches the family_string_lookup[] array for the fpga family
 * name string associated with a specific version code. The fpga family name is
 * returned if a match is found, otherwise "(unknown)".
 *
 * @code: fpga family version code
 *
 * Return: Pointer to the fpga family name if the entry is found, otherwise
 * "(unknown)".
 */
const char *microblaze_lookup_fpga_family_string(const u32 code);

/**
 * microblaze_early_cpuinfo_init() - Initialize cpuinfo with default values.
 *
 * Initializes the global data cpuinfo structure with default values (cache
 * size, cache line size, etc.). It is called very early in the boot process
 * (start.S codepath right before the first cache flush call) to ensure that
 * cache related operations are properly handled.
 */
void microblaze_early_cpuinfo_init(void);

#endif	/* __ASM_MICROBLAZE_CPUINFO_H */
