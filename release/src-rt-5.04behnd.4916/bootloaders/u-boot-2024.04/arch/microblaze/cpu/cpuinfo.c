// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022, Ovidiu Panait <ovpanait@gmail.com>
 */
#include <asm/cpuinfo.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

#if IS_ENABLED(CONFIG_CPU_MICROBLAZE)
/* These key value are as per MBV field in PVR0 */
static const struct microblaze_version_map cpu_ver_lookup[] = {
	{"5.00.a", 0x01},
	{"5.00.b", 0x02},
	{"5.00.c", 0x03},
	{"6.00.a", 0x04},
	{"6.00.b", 0x06},
	{"7.00.a", 0x05},
	{"7.00.b", 0x07},
	{"7.10.a", 0x08},
	{"7.10.b", 0x09},
	{"7.10.c", 0x0a},
	{"7.10.d", 0x0b},
	{"7.20.a", 0x0c},
	{"7.20.b", 0x0d},
	{"7.20.c", 0x0e},
	{"7.20.d", 0x0f},
	{"7.30.a", 0x10},
	{"7.30.b", 0x11},
	{"8.00.a", 0x12},
	{"8.00.b", 0x13},
	{"8.10.a", 0x14},
	{"8.20.a", 0x15},
	{"8.20.b", 0x16},
	{"8.30.a", 0x17},
	{"8.40.a", 0x18},
	{"8.40.b", 0x19},
	{"8.50.a", 0x1a},
	{"8.50.b", 0x1c},
	{"8.50.c", 0x1e},
	{"9.0", 0x1b},
	{"9.1", 0x1d},
	{"9.2", 0x1f},
	{"9.3", 0x20},
	{"9.4", 0x21},
	{"9.5", 0x22},
	{"9.6", 0x23},
	{"10.0", 0x24},
	{"11.0", 0x25},
	{NULL, 0},
};

static const struct microblaze_version_map family_string_lookup[] = {
	{"virtex2", 0x4},
	{"virtex2pro", 0x5},
	{"spartan3", 0x6},
	{"virtex4", 0x7},
	{"virtex5", 0x8},
	{"spartan3e", 0x9},
	{"spartan3a", 0xa},
	{"spartan3an", 0xb},
	{"spartan3adsp", 0xc},
	{"spartan6", 0xd},
	{"virtex6", 0xe},
	{"virtex7", 0xf},
	/* FIXME There is no key code defined for spartan2 */
	{"spartan2", 0xf0},
	{"kintex7", 0x10},
	{"artix7", 0x11},
	{"zynq7000", 0x12},
	{"UltraScale Virtex", 0x13},
	{"UltraScale Kintex", 0x14},
	{"UltraScale+ Zynq", 0x15},
	{"UltraScale+ Virtex", 0x16},
	{"UltraScale+ Kintex", 0x17},
	{"Spartan7", 0x18},
	{NULL, 0},
};

static const char *lookup_string(u32 code,
				 const struct microblaze_version_map *entry)
{
	for (; entry->string; ++entry)
		if (entry->code == code)
			return entry->string;

	return "(unknown)";
}

static const u32 lookup_code(const char *string,
			     const struct microblaze_version_map *entry)
{
	for (; entry->string; ++entry)
		if (!strcmp(entry->string, string))
			return entry->code;

	return 0;
}

const char *microblaze_lookup_fpga_family_string(const u32 code)
{
	return lookup_string(code, family_string_lookup);
}

const char *microblaze_lookup_cpu_version_string(const u32 code)
{
	return lookup_string(code, cpu_ver_lookup);
}

const u32 microblaze_lookup_fpga_family_code(const char *string)
{
	return lookup_code(string, family_string_lookup);
}

const u32 microblaze_lookup_cpu_version_code(const char *string)
{
	return lookup_code(string, cpu_ver_lookup);
}
#endif /* CONFIG_CPU_MICROBLAZE */

void microblaze_early_cpuinfo_init(void)
{
	struct microblaze_cpuinfo *ci = gd_cpuinfo();

	ci->icache_size = CONFIG_XILINX_MICROBLAZE0_ICACHE_SIZE;
	ci->icache_line_length = 4;

	ci->dcache_size = CONFIG_XILINX_MICROBLAZE0_DCACHE_SIZE;
	ci->dcache_line_length = 4;
}
