// SPDX-License-Identifier: GPL-2.0
/*
 * From Coreboot src/northbridge/intel/sandybridge/report_platform.c
 *
 * Copyright (C) 2012 Google Inc.
 */

#include <common.h>
#include <log.h>
#include <asm/cpu.h>
#include <asm/pci.h>
#include <asm/report_platform.h>
#include <asm/arch/pch.h>

static void report_cpu_info(void)
{
	char cpu_string[CPU_MAX_NAME_LEN], *cpu_name;
	const char *mode[] = {"NOT ", ""};
	struct cpuid_result cpuidr;
	int vt, txt, aes;
	u32 index;

	index = 0x80000000;
	cpuidr = cpuid(index);
	if (cpuidr.eax < 0x80000004) {
		strcpy(cpu_string, "Platform info not available");
		cpu_name = cpu_string;
	} else {
		cpu_name = cpu_get_name(cpu_string);
	}

	cpuidr = cpuid(1);
	debug("CPU id(%x): %s\n", cpuidr.eax, cpu_name);
	aes = (cpuidr.ecx & (1 << 25)) ? 1 : 0;
	txt = (cpuidr.ecx & (1 << 6)) ? 1 : 0;
	vt = (cpuidr.ecx & (1 << 5)) ? 1 : 0;
	debug("AES %ssupported, TXT %ssupported, VT %ssupported\n",
	      mode[aes], mode[txt], mode[vt]);
}

/* The PCI id name match comes from Intel document 472178 */
static struct {
	u16 dev_id;
	const char *dev_name;
} pch_table[] = {
	{0x1E41, "Desktop Sample"},
	{0x1E42, "Mobile Sample"},
	{0x1E43, "SFF Sample"},
	{0x1E44, "Z77"},
	{0x1E45, "H71"},
	{0x1E46, "Z75"},
	{0x1E47, "Q77"},
	{0x1E48, "Q75"},
	{0x1E49, "B75"},
	{0x1E4A, "H77"},
	{0x1E53, "C216"},
	{0x1E55, "QM77"},
	{0x1E56, "QS77"},
	{0x1E58, "UM77"},
	{0x1E57, "HM77"},
	{0x1E59, "HM76"},
	{0x1E5D, "HM75"},
	{0x1E5E, "HM70"},
	{0x1E5F, "NM70"},
};

static void report_pch_info(struct udevice *dev)
{
	const char *pch_type = "Unknown";
	int i;
	u16 dev_id;
	uint8_t rev_id;

	dm_pci_read_config16(dev, 2, &dev_id);
	for (i = 0; i < ARRAY_SIZE(pch_table); i++) {
		if (pch_table[i].dev_id == dev_id) {
			pch_type = pch_table[i].dev_name;
			break;
		}
	}
	dm_pci_read_config8(dev, 8, &rev_id);
	debug("PCH type: %s, device id: %x, rev id %x\n", pch_type, dev_id,
	      rev_id);
}

void report_platform_info(struct udevice *dev)
{
	report_cpu_info();
	report_pch_info(dev);
}
