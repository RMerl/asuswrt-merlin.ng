// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015, Bin Meng <bmeng.cn@gmail.com>
 *
 * Adapted from coreboot src/arch/x86/smbios.c
 */

#include <common.h>
#include <mapmem.h>
#include <smbios.h>
#include <tables_csum.h>
#include <version.h>
#ifdef CONFIG_CPU
#include <cpu.h>
#include <dm.h>
#include <dm/uclass-internal.h>
#endif

/**
 * smbios_add_string() - add a string to the string area
 *
 * This adds a string to the string area which is appended directly after
 * the formatted portion of an SMBIOS structure.
 *
 * @start:	string area start address
 * @str:	string to add
 * @return:	string number in the string area
 */
static int smbios_add_string(char *start, const char *str)
{
	int i = 1;
	char *p = start;

	for (;;) {
		if (!*p) {
			strcpy(p, str);
			p += strlen(str);
			*p++ = '\0';
			*p++ = '\0';

			return i;
		}

		if (!strcmp(p, str))
			return i;

		p += strlen(p) + 1;
		i++;
	}
}

/**
 * smbios_string_table_len() - compute the string area size
 *
 * This computes the size of the string area including the string terminator.
 *
 * @start:	string area start address
 * @return:	string area size
 */
static int smbios_string_table_len(char *start)
{
	char *p = start;
	int i, len = 0;

	while (*p) {
		i = strlen(p) + 1;
		p += i;
		len += i;
	}

	return len + 1;
}

static int smbios_write_type0(ulong *current, int handle)
{
	struct smbios_type0 *t;
	int len = sizeof(struct smbios_type0);

	t = map_sysmem(*current, len);
	memset(t, 0, sizeof(struct smbios_type0));
	fill_smbios_header(t, SMBIOS_BIOS_INFORMATION, len, handle);
	t->vendor = smbios_add_string(t->eos, "U-Boot");
	t->bios_ver = smbios_add_string(t->eos, PLAIN_VERSION);
	t->bios_release_date = smbios_add_string(t->eos, U_BOOT_DMI_DATE);
#ifdef CONFIG_ROM_SIZE
	t->bios_rom_size = (CONFIG_ROM_SIZE / 65536) - 1;
#endif
	t->bios_characteristics = BIOS_CHARACTERISTICS_PCI_SUPPORTED |
				  BIOS_CHARACTERISTICS_SELECTABLE_BOOT |
				  BIOS_CHARACTERISTICS_UPGRADEABLE;
#ifdef CONFIG_GENERATE_ACPI_TABLE
	t->bios_characteristics_ext1 = BIOS_CHARACTERISTICS_EXT1_ACPI;
#endif
#ifdef CONFIG_EFI_LOADER
	t->bios_characteristics_ext1 |= BIOS_CHARACTERISTICS_EXT1_UEFI;
#endif
	t->bios_characteristics_ext2 = BIOS_CHARACTERISTICS_EXT2_TARGET;

	t->bios_major_release = 0xff;
	t->bios_minor_release = 0xff;
	t->ec_major_release = 0xff;
	t->ec_minor_release = 0xff;

	len = t->length + smbios_string_table_len(t->eos);
	*current += len;
	unmap_sysmem(t);

	return len;
}

static int smbios_write_type1(ulong *current, int handle)
{
	struct smbios_type1 *t;
	int len = sizeof(struct smbios_type1);
	char *serial_str = env_get("serial#");

	t = map_sysmem(*current, len);
	memset(t, 0, sizeof(struct smbios_type1));
	fill_smbios_header(t, SMBIOS_SYSTEM_INFORMATION, len, handle);
	t->manufacturer = smbios_add_string(t->eos, CONFIG_SMBIOS_MANUFACTURER);
	t->product_name = smbios_add_string(t->eos, CONFIG_SMBIOS_PRODUCT_NAME);
	if (serial_str) {
		strncpy((char *)t->uuid, serial_str, sizeof(t->uuid));
		t->serial_number = smbios_add_string(t->eos, serial_str);
	}

	len = t->length + smbios_string_table_len(t->eos);
	*current += len;
	unmap_sysmem(t);

	return len;
}

static int smbios_write_type2(ulong *current, int handle)
{
	struct smbios_type2 *t;
	int len = sizeof(struct smbios_type2);

	t = map_sysmem(*current, len);
	memset(t, 0, sizeof(struct smbios_type2));
	fill_smbios_header(t, SMBIOS_BOARD_INFORMATION, len, handle);
	t->manufacturer = smbios_add_string(t->eos, CONFIG_SMBIOS_MANUFACTURER);
	t->product_name = smbios_add_string(t->eos, CONFIG_SMBIOS_PRODUCT_NAME);
	t->feature_flags = SMBIOS_BOARD_FEATURE_HOSTING;
	t->board_type = SMBIOS_BOARD_MOTHERBOARD;

	len = t->length + smbios_string_table_len(t->eos);
	*current += len;
	unmap_sysmem(t);

	return len;
}

static int smbios_write_type3(ulong *current, int handle)
{
	struct smbios_type3 *t;
	int len = sizeof(struct smbios_type3);

	t = map_sysmem(*current, len);
	memset(t, 0, sizeof(struct smbios_type3));
	fill_smbios_header(t, SMBIOS_SYSTEM_ENCLOSURE, len, handle);
	t->manufacturer = smbios_add_string(t->eos, CONFIG_SMBIOS_MANUFACTURER);
	t->chassis_type = SMBIOS_ENCLOSURE_DESKTOP;
	t->bootup_state = SMBIOS_STATE_SAFE;
	t->power_supply_state = SMBIOS_STATE_SAFE;
	t->thermal_state = SMBIOS_STATE_SAFE;
	t->security_status = SMBIOS_SECURITY_NONE;

	len = t->length + smbios_string_table_len(t->eos);
	*current += len;
	unmap_sysmem(t);

	return len;
}

static void smbios_write_type4_dm(struct smbios_type4 *t)
{
	u16 processor_family = SMBIOS_PROCESSOR_FAMILY_UNKNOWN;
	const char *vendor = "Unknown";
	const char *name = "Unknown";

#ifdef CONFIG_CPU
	char processor_name[49];
	char vendor_name[49];
	struct udevice *dev = NULL;

	uclass_find_first_device(UCLASS_CPU, &dev);
	if (dev) {
		struct cpu_platdata *plat = dev_get_parent_platdata(dev);

		if (plat->family)
			processor_family = plat->family;
		t->processor_id[0] = plat->id[0];
		t->processor_id[1] = plat->id[1];

		if (!cpu_get_vendor(dev, vendor_name, sizeof(vendor_name)))
			vendor = vendor_name;
		if (!cpu_get_desc(dev, processor_name, sizeof(processor_name)))
			name = processor_name;
	}
#endif

	t->processor_family = processor_family;
	t->processor_manufacturer = smbios_add_string(t->eos, vendor);
	t->processor_version = smbios_add_string(t->eos, name);
}

static int smbios_write_type4(ulong *current, int handle)
{
	struct smbios_type4 *t;
	int len = sizeof(struct smbios_type4);

	t = map_sysmem(*current, len);
	memset(t, 0, sizeof(struct smbios_type4));
	fill_smbios_header(t, SMBIOS_PROCESSOR_INFORMATION, len, handle);
	t->processor_type = SMBIOS_PROCESSOR_TYPE_CENTRAL;
	smbios_write_type4_dm(t);
	t->status = SMBIOS_PROCESSOR_STATUS_ENABLED;
	t->processor_upgrade = SMBIOS_PROCESSOR_UPGRADE_NONE;
	t->l1_cache_handle = 0xffff;
	t->l2_cache_handle = 0xffff;
	t->l3_cache_handle = 0xffff;
	t->processor_family2 = t->processor_family;

	len = t->length + smbios_string_table_len(t->eos);
	*current += len;
	unmap_sysmem(t);

	return len;
}

static int smbios_write_type32(ulong *current, int handle)
{
	struct smbios_type32 *t;
	int len = sizeof(struct smbios_type32);

	t = map_sysmem(*current, len);
	memset(t, 0, sizeof(struct smbios_type32));
	fill_smbios_header(t, SMBIOS_SYSTEM_BOOT_INFORMATION, len, handle);

	*current += len;
	unmap_sysmem(t);

	return len;
}

static int smbios_write_type127(ulong *current, int handle)
{
	struct smbios_type127 *t;
	int len = sizeof(struct smbios_type127);

	t = map_sysmem(*current, len);
	memset(t, 0, sizeof(struct smbios_type127));
	fill_smbios_header(t, SMBIOS_END_OF_TABLE, len, handle);

	*current += len;
	unmap_sysmem(t);

	return len;
}

static smbios_write_type smbios_write_funcs[] = {
	smbios_write_type0,
	smbios_write_type1,
	smbios_write_type2,
	smbios_write_type3,
	smbios_write_type4,
	smbios_write_type32,
	smbios_write_type127
};

ulong write_smbios_table(ulong addr)
{
	struct smbios_entry *se;
	ulong table_addr;
	ulong tables;
	int len = 0;
	int max_struct_size = 0;
	int handle = 0;
	char *istart;
	int isize;
	int i;

	/* 16 byte align the table address */
	addr = ALIGN(addr, 16);

	se = map_sysmem(addr, sizeof(struct smbios_entry));
	memset(se, 0, sizeof(struct smbios_entry));

	addr += sizeof(struct smbios_entry);
	addr = ALIGN(addr, 16);
	tables = addr;

	/* populate minimum required tables */
	for (i = 0; i < ARRAY_SIZE(smbios_write_funcs); i++) {
		int tmp = smbios_write_funcs[i]((ulong *)&addr, handle++);

		max_struct_size = max(max_struct_size, tmp);
		len += tmp;
	}

	memcpy(se->anchor, "_SM_", 4);
	se->length = sizeof(struct smbios_entry);
	se->major_ver = SMBIOS_MAJOR_VER;
	se->minor_ver = SMBIOS_MINOR_VER;
	se->max_struct_size = max_struct_size;
	memcpy(se->intermediate_anchor, "_DMI_", 5);
	se->struct_table_length = len;

	/*
	 * We must use a pointer here so things work correctly on sandbox. The
	 * user of this table is not aware of the mapping of addresses to
	 * sandbox's DRAM buffer.
	 */
	table_addr = (ulong)map_sysmem(tables, 0);
	if (sizeof(table_addr) > sizeof(u32) && table_addr > (ulong)UINT_MAX) {
		/*
		 * We need to put this >32-bit pointer into the table but the
		 * field is only 32 bits wide.
		 */
		printf("WARNING: SMBIOS table_address overflow %llx\n",
		       (unsigned long long)table_addr);
		table_addr = 0;
	}
	se->struct_table_address = table_addr;

	se->struct_count = handle;

	/* calculate checksums */
	istart = (char *)se + SMBIOS_INTERMEDIATE_OFFSET;
	isize = sizeof(struct smbios_entry) - SMBIOS_INTERMEDIATE_OFFSET;
	se->intermediate_checksum = table_compute_checksum(istart, isize);
	se->checksum = table_compute_checksum(se, sizeof(struct smbios_entry));
	unmap_sysmem(se);

	return addr;
}
